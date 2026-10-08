#include "falcon-routine/hub.hpp"
#include "falcon-database/DatabaseConnection.hpp"
#include <atomic>
#include <exception>
#include <falcon-comms/routine_comms.hpp>
#include <falcon-comms/runtime_comms.hpp>
#include <falcon-core/communications/Time.hpp>
#include <falcon-core/communications/messages/MeasurementRequest.hpp>
#include <falcon-core/communications/messages/MeasurementResponse.hpp>
#include <falcon-core/communications/messages/SettingRequest.hpp>
#include <falcon-core/communications/messages/SettingResponse.hpp>
#include <falcon-core/communications/messages/VoltageStatesResponse.hpp>
#include <falcon-core/instrument_interfaces/names/InstrumentPort.hpp>
#include <falcon-core/math/Vector.hpp>
#include <falcon-core/physics/config/core/VoltageConstraints.hpp>
#include <future>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>
namespace {
constexpr std::string_view DEVICE_CACHE_SCOPE = "cache";
constexpr std::string_view DEVICE_VOLTAGES_CACHE_NAME = "device_voltages";
constexpr std::string_view OHMICS_CONNECTED_TO_VOLTAGE_SOURCES_CACHE_NAME =
    "ohmics_connected_to_voltage_sources";
constexpr std::string_view CONFIG_CACHE_NAME = "config";
namespace db = falcon::database;
void cache_item(std::string_view name, std::string value) {
  db::ReadWriteDatabaseConnection db_conn;
  db_conn.delete_by_name(std::string(name));

  db::DeviceCharacteristic dchar;
  dchar.scope = DEVICE_CACHE_SCOPE;
  dchar.name = name;
  dchar.characteristic = value;
  db_conn.insert(dchar);
}
std::vector<db::DeviceCharacteristic> read_cache(std::string_view name) {
  db::DeviceCharacteristicQuery query;
  db::ReadOnlyDatabaseConnection db_conn;
  query.scope = DEVICE_CACHE_SCOPE;
  query.name = name;
  return db_conn.get_by_query(query);
}

void cache_config(const falcon_core::physics::config::core::ConfigSP &config) {
  cache_item(CONFIG_CACHE_NAME, config->to_json_string());
}

void cache_device_voltages(
    const falcon_core::communications::voltage_states::DeviceVoltageStatesSP
        &voltages) {
  cache_item(DEVICE_VOLTAGES_CACHE_NAME, voltages->to_json_string());
}
} // namespace

namespace falcon::routine {
using falcon_core::communications::Time;

falcon_core::communications::messages::VoltageStatesResponseSP
request_device_state(int timeout_ms) {
  falcon::comms::RoutineComms comms;
  long long value = Time().time();
  auto resp = comms.subscribe_state_response(timeout_ms, value);
  return falcon_core::communications::messages::VoltageStatesResponse::
      from_json_string<
          falcon_core::communications::messages::VoltageStatesResponse>(
          resp.response);
}
using falcon_core::communications::messages::MeasurementResponse;
using falcon_core::communications::messages::MeasurementResponseSP;
MeasurementResponseSP request_measurement(
    const falcon_core::communications::messages::MeasurementRequestSP &req,
    int timeout_ms) {
  falcon::comms::RoutineComms comms;
  std::string json_req = req->to_json_string();
  long long value = Time().time();
  auto resp = comms.subscribe_measure_response(json_req, timeout_ms, value);
  auto outs = comms.pull_measurement_data(resp.stream, resp.channel, 1);
  if (outs.empty()) {
    throw std::runtime_error("No measurement data received");
  }
  return MeasurementResponse::from_json_string<MeasurementResponse>(
      outs.front());
}

using falcon_core::communications::messages::SettingResponse;
using falcon_core::communications::messages::SettingResponseSP;
SettingResponseSP request_setting(
    const falcon_core::communications::messages::SettingRequestSP &req,
    int timeout_ms) {
  auto &hub = falcon::comms::NatsManager::instance();
  std::promise<std::string> prom;
  auto fut = prom.get_future();
  std::atomic<bool> done{false};

  long long timestamp = Time().time();
  std::string response_subject =
      "FALCON.SETTING_RESPONSE." + std::to_string(timestamp);

  hub.subscribe(response_subject, [&prom, &done](const std::string &data) {
    if (done.exchange(true)) {
      return;
    }
    prom.set_value(data);
  });

  nlohmann::json cmd = {
      {"timestamp", timestamp},
      {"request", req->to_json_string()},
  };
  hub.publish("INSTRUMENTHUB.SETTING_COMMAND", cmd.dump());

  try {
    if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) ==
        std::future_status::ready) {
      auto result = fut.get();
      hub.unsubscribe(response_subject);
      auto json = nlohmann::json::parse(result);
      std::string resp_str;
      if (json.is_object() && json.contains("response")) {
        resp_str = json.at("response").get<std::string>();
      } else {
        resp_str = result;
      }
      return SettingResponse::from_json_string<SettingResponse>(resp_str);
    }

    done = true;
    hub.unsubscribe(response_subject);
    throw std::runtime_error("Timeout waiting for SettingResponse");
  } catch (...) {
    hub.unsubscribe(response_subject);
    throw;
  }
}

using falcon_core::physics::config::core::Config;
using falcon_core::physics::config::core::ConfigSP;
ConfigSP request_config(int timeout_ms) {
  falcon::comms::RuntimeComms comms;
  long long value = Time().time();
  auto resp = comms.subscribe_config_response(timeout_ms, value);
  return Config::from_json_string<Config>(resp.response);
}

using falcon_core::instrument_interfaces::names::Ports;
using falcon_core::instrument_interfaces::names::PortsSP;
std::tuple<PortsSP, PortsSP, PortsSP> request_port_payload(int timeout_ms) {
  falcon::comms::RuntimeComms comms;
  long long value = Time().time();
  auto resp = comms.subscribe_port_payload(timeout_ms, value);
  PortsSP knobs = Ports::from_json_string<Ports>(resp.knobs);
  PortsSP meters = Ports::from_json_string<Ports>(resp.meters);
  PortsSP settings = Ports::from_json_string<Ports>(resp.settings);
  return {knobs, meters, settings};
}

falcon_core::communications::voltage_states::DeviceVoltageStatesSP
read_device_voltages(int timeout_ms) {
  auto results = read_cache(DEVICE_VOLTAGES_CACHE_NAME);
  if (!results.empty()) {
    // Cache hit, return cached value
    return falcon_core::communications::voltage_states::DeviceVoltageStates::
        from_json_string<
            falcon_core::communications::voltage_states::DeviceVoltageStates>(
            results.front().characteristic);
  }
  auto voltages = request_device_state(timeout_ms)->states();
  std::thread([voltages] { cache_device_voltages(voltages); }).detach();
  return voltages;
}
ConfigSP read_config(int timeout_ms) {
  auto results = read_cache(CONFIG_CACHE_NAME);
  if (!results.empty()) {
    // Cache hit, return cached value
    return Config::from_json_string<Config>(results.front().characteristic);
  }
  auto config = request_config(timeout_ms);
  std::thread([config] { cache_config(config); }).detach();
  return config;
}

using falcon_core::generic::List;
using falcon_core::instrument_interfaces::names::InstrumentPort;
using falcon_core::instrument_interfaces::names::InstrumentPortSP;
using falcon_core::physics::device_structures::Connections;
using falcon_core::physics::device_structures::ConnectionsSP;
ConnectionsSP get_ohmics_connected_to_voltage_sources(int timeout_ms) {
  auto results = read_cache(OHMICS_CONNECTED_TO_VOLTAGE_SOURCES_CACHE_NAME);
  if (!results.empty()) {
    // Cache hit, return cached value
    return Connections::from_json_string<Connections>(
        results.front().characteristic);
  }
  auto payload = request_port_payload(timeout_ms);
  PortsSP knobs = std::get<0>(payload);
  PortsSP meters = std::get<1>(payload);

  ConfigSP config = request_config(timeout_ms);
  ConnectionsSP connections = config->ohmics();
  ConnectionsSP ohmics_connected_to_voltage_sources =
      std::make_shared<Connections>();
  List<InstrumentPort> raw_ports = *knobs->ports();
  Connections raw_ohmics = *connections;
  for (const InstrumentPortSP &knob : raw_ports) {
    auto knob_connection = knob->pseudo_name();
    if (knob_connection->is_ohmic()) {
      for (const auto &ohmic : raw_ohmics) {
        if (*knob_connection == *ohmic) {
          ohmics_connected_to_voltage_sources->push_back(ohmic);
        }
      }
    }
  }
  // Remove duplicates to ensure uniqueness
  std::sort(ohmics_connected_to_voltage_sources->begin(),
            ohmics_connected_to_voltage_sources->end());

  if (ohmics_connected_to_voltage_sources->size() > 1) {
    for (size_t i = ohmics_connected_to_voltage_sources->size() - 1; i > 0;
         --i) {
      if (ohmics_connected_to_voltage_sources->at(i) ==
          ohmics_connected_to_voltage_sources->at(i - 1)) {
        ohmics_connected_to_voltage_sources->erase_at(i);
      }
    }
  }
  // Launch cache update asynchronously
  std::thread([ohmics_connected_to_voltage_sources] {
    cache_item(OHMICS_CONNECTED_TO_VOLTAGE_SOURCES_CACHE_NAME,
               ohmics_connected_to_voltage_sources->to_json_string());
  }).detach();
  return ohmics_connected_to_voltage_sources;
}

physics::device_structures::GateRelationsSP get_gate_relations(int timeout_ms) {
  auto config = request_config(timeout_ms);
  return config->generate_gate_relations();
}

math::domains::CoupledLabelledDomainSP
get_voltage_bounds(const instrument_interfaces::names::PortsSP &search_domain,
                   int timeout_ms) {
  auto voltage_states = read_device_voltages(timeout_ms);
  if (!voltage_states) {
    throw std::runtime_error("Failed to read device voltages");
  }
  auto config = read_config(timeout_ms);
  if (!config) {
    throw std::runtime_error("Failed to read config");
  }
  physics::config::core::VoltageConstraints constraints{
      config->adjacency(), config->max_safe_diff(),
      std::make_pair<double, double>(config->min_bound(), config->max_bound())};

  return constraints.compute_maximal_domain(search_domain, voltage_states);
}
bool safe_voltage_change(math::PointSP proposed_voltages, int timeout_ms) {
  auto config = read_config(timeout_ms);
  if (!config) {
    throw std::runtime_error("Failed to read config");
  }
  physics::config::core::VoltageConstraints constraints{
      config->adjacency(), config->max_safe_diff(),
      std::make_pair<double, double>(config->min_bound(), config->max_bound())};

  return constraints.validate_voltage_state(std::move(proposed_voltages));
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool ramp(const math::PointSP &end_point, double max_ramp_rate,
          int timeout_ms) {
  safe_voltage_change(end_point, timeout_ms);
  auto start_point = read_device_voltages(timeout_ms)->to_point();
  math::Vector sweep_vector(start_point, end_point);
  auto principal_axis = sweep_vector.principle_connection();
  generic::PairSP<math::Quantity, math::Quantity> principal_bounds =
      sweep_vector.at(principal_axis);
  double total_time = std::abs(
      std::abs(
          (*principal_bounds->first() - principal_bounds->second())->value()) /
      max_ramp_rate);
  std::tuple<PortsSP, PortsSP, PortsSP> payload =
      request_port_payload(timeout_ms);
  PortsSP knobs = std::get<0>(payload);
  auto time_domain = math::domains::LabelledDomain::from_port(
      std::make_pair(0.0, total_time), InstrumentPort::Timer());
  List<physics::device_structures::Connection> connections =
      *end_point->connections();
  auto increasing = std::make_shared<generic::Map<std::string, bool>>();
  math::domains::CoupledLabelledDomainSP domains =
      std::make_shared<math::domains::CoupledLabelledDomain>();
  for (const physics::device_structures::ConnectionSP &connection :
       connections) {
    auto itr =
        std::find_if(knobs->begin(), knobs->end(), [&](const auto &raw_knob) {
          return (*raw_knob->pseudo_name() == *connection) &&
                 (raw_knob->instrument() ==
                      falcon_core::instrument_interfaces::names::Instrument::
                          DC_Voltage_Source ||
                  raw_knob->instrument() ==
                      falcon_core::instrument_interfaces::names::Instrument::
                          Voltage_Source);
        });
    if (itr == knobs->end()) {
      throw std::runtime_error(
          "No connection was found in the ports matching the request");
    }
    const auto &found_knob = *itr;
    domains->push_back(math::domains::LabelledDomain::from_port(
        std::make_pair(start_point->at(connection)->value(),
                       end_point->at(connection)->value()),
        found_knob));
    increasing->insert(connection->name(), (end_point->at(connection) >
                                            start_point->at(connection)));
  }
  // Division 1 sweep is a ramp always. The time domain here pertains to the
  // whole measurement time of the ramp
  std::vector<instrument_interfaces::WaveformSP> raw_waveform = {
      instrument_interfaces::Waveform::CartesianIdentityWaveform1D(1, domains,
                                                                   increasing),
  };
  auto waveforms =
      std::make_shared<generic::List<instrument_interfaces::Waveform>>(
          raw_waveform);
  auto request = std::make_shared<communications::messages::MeasurementRequest>(
      "Performing a ramp measurement", waveforms, std::make_shared<Ports>(),
      std::make_shared<generic::Map<
          InstrumentPort,
          instrument_interfaces::port_transforms::PortTransform>>(),
      time_domain);

  try {
    request_measurement(request, timeout_ms);
    return true;
  } catch (const std::exception &e) {
    return false;
  }
}
} // namespace falcon::routine
