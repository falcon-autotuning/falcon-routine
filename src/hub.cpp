#include "falcon-routine/hub.hpp"
#include "falcon-routine/database.hpp"
#include <falcon-comms/routine_comms.hpp>
#include <falcon-comms/runtime_comms.hpp>
#include <falcon-core/communications/Time.hpp>
#include <falcon-core/communications/messages/VoltageStatesResponse.hpp>
#include <falcon-core/physics/config/core/VoltageConstraints.hpp>
#include <limits>
#include <stdexcept>
#include <thread>
#include <vector>
namespace {
const char *DEVICE_CACHE_SCOPE = "cache";
namespace db = falcon::database;
void cache_item(std::string name, std::string value) {
  db::DeviceCharacteristic dc;
  dc.scope = DEVICE_CACHE_SCOPE;
  dc.name = name;
  dc.characteristic = value;
  db::ReadWriteDatabaseConnection db_conn;
  db_conn.insert(dc);
}
std::vector<db::DeviceCharacteristic> read_cache(std::string name) {
  db::DeviceCharacteristicQuery query;
  db::ReadOnlyDatabaseConnection db_conn;
  query.scope = DEVICE_CACHE_SCOPE;
  query.name = name;
  return db_conn.get_by_query(query);
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

falcon_core::communications::messages::MeasurementResponseSP
request_measurement(
    const falcon_core::communications::messages::MeasurementRequestSP &req,
    int timeout_ms) {
  falcon::comms::RoutineComms comms;
  std::string json_req = req->to_json_string();
  long long value = Time().time();
  auto resp = comms.subscribe_measure_response(json_req, timeout_ms, value);
  auto outs = comms.pull_measurement_data(resp.stream, resp.channel, 1);
  return falcon_core::communications::messages::MeasurementResponse::
      from_json_string<
          falcon_core::communications::messages::MeasurementResponse>(outs[0]);
}

falcon_core::physics::config::core::ConfigSP request_config(int timeout_ms) {
  falcon::comms::RuntimeComms comms;
  long long value = Time().time();
  auto resp = comms.subscribe_config_response(timeout_ms, value);
  return falcon_core::physics::config::core::Config::from_json_string<
      falcon_core::physics::config::core::Config>(resp.response);
}
std::tuple<falcon_core::instrument_interfaces::names::Ports,
           falcon_core::instrument_interfaces::names::Ports>
request_port_payload(int timeout_ms) {
  falcon::comms::RuntimeComms comms;
  long long value = Time().time();
  auto resp = comms.subscribe_port_payload(timeout_ms, value);
  auto knobs =
      falcon_core::instrument_interfaces::names::Ports::from_json_string<
          falcon_core::instrument_interfaces::names::Ports>(resp.knobs);
  auto meters =
      falcon_core::instrument_interfaces::names::Ports::from_json_string<
          falcon_core::instrument_interfaces::names::Ports>(resp.meters);
  return std::tuple<falcon_core::instrument_interfaces::names::Ports,
                    falcon_core::instrument_interfaces::names::Ports>(knobs,
                                                                      meters);
}

const char *DEVICE_VOLTAGES_CACHE_NAME = "device_voltages";
const char *OHMICS_CONNECTED_TO_VOLTAGE_SOURCES_CACHE_NAME =
    "ohmics_connected_to_voltage_sources";
const char *CONFIG_CACHE_NAME = "config";
void cache_device_voltages(
    falcon_core::communications::voltage_states::DeviceVoltageStatesSP
        voltages) {
  return cache_item(DEVICE_VOLTAGES_CACHE_NAME, voltages->to_json_string());
}
falcon_core::communications::voltage_states::DeviceVoltageStatesSP
read_device_voltages(int timeout_ms) {
  auto results = read_cache(DEVICE_VOLTAGES_CACHE_NAME);
  if (!results.empty()) {
    // Cache hit, return cached value
    return falcon_core::communications::voltage_states::DeviceVoltageStates::
        from_json_string<
            falcon_core::communications::voltage_states::DeviceVoltageStates>(
            results[0].characteristic);
  }
  auto voltages = request_device_state(timeout_ms)->states();
  std::thread([voltages] { cache_device_voltages(voltages); }).detach();
  return voltages;
}
void cache_config(physics::config::core::ConfigSP config) {
  return cache_item(CONFIG_CACHE_NAME, config->to_json_string());
}
physics::config::core::ConfigSP read_config(int timeout_ms) {
  auto results = read_cache(CONFIG_CACHE_NAME);
  if (!results.empty()) {
    // Cache hit, return cached value
    return falcon_core::physics::config::core::Config::from_json_string<
        falcon_core::physics::config::core::Config>(results[0].characteristic);
  }
  auto config = request_config(timeout_ms);
  std::thread([config] { cache_config(config); }).detach();
  return config;
}

falcon_core::physics::device_structures::ConnectionsSP
get_ohmics_connected_to_voltage_sources(int timeout_ms) {
  auto results = read_cache(OHMICS_CONNECTED_TO_VOLTAGE_SOURCES_CACHE_NAME);
  if (!results.empty()) {
    // Cache hit, return cached value
    return falcon_core::physics::device_structures::Connections::
        from_json_string<falcon_core::physics::device_structures::Connections>(
            results[0].characteristic);
  }
  auto payload = request_port_payload(timeout_ms);
  auto knobs = std::get<0>(payload);
  auto meters = std::get<1>(payload);

  auto config = request_config(timeout_ms);
  auto connections = config->ohmics();
  physics::device_structures::ConnectionsSP
      ohmics_connected_to_voltage_sources =
          std::make_shared<physics::device_structures::Connections>();
  auto raw_ports = *knobs.ports();
  auto raw_ohmics = *connections;
  for (const auto &knob : raw_ports) {
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

  for (size_t i = ohmics_connected_to_voltage_sources->size() - 1; i > 0; --i) {
    if ((*ohmics_connected_to_voltage_sources)[i] ==
        (*ohmics_connected_to_voltage_sources)[i - 1]) {
      ohmics_connected_to_voltage_sources->erase_at(i);
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
get_voltage_bounds(const instrument_interfaces::names::PortsSP search_domain,
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
bool FALCON_ROUTINE_API safe_voltage_change(math::PointSP proposed_voltages,
                                            int timeout_ms) {
  auto config = read_config(timeout_ms);
  if (!config) {
    throw std::runtime_error("Failed to read config");
  }
  physics::config::core::VoltageConstraints constraints{
      config->adjacency(), config->max_safe_diff(),
      std::make_pair<double, double>(config->min_bound(), config->max_bound())};

  return constraints.validate_voltage_state(proposed_voltages);
}
} // namespace falcon::routine
