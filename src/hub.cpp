#include "falcon-routine/hub.hpp"
#include "falcon-routine/database.hpp"
#include <falcon-comms/routine_comms.hpp>
#include <falcon-comms/runtime_comms.hpp>
#include <falcon-core/communications/Time.hpp>
#include <falcon-core/communications/messages/VoltageStatesResponse.hpp>
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
  db::LazyReadWriteDatabaseConnection lazy_db;
  lazy_db->insert(dc);
}
std::vector<db::DeviceCharacteristic> read_cache(std::string name) {
  db::DeviceCharacteristicQuery query;
  db::LazyReadOnlyDatabaseConnection lazy_db;
  query.scope = DEVICE_CACHE_SCOPE;
  query.name = name;
  return lazy_db->get_by_query(query);
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
  auto voltages = request_device_state(timeout_ms)->states;
  std::thread([voltages] { cache_device_voltages(voltages); }).detach();
  return voltages;
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
  auto connections = config->device_structure->connections;
  physics::device_structures::ConnectionsSP
      ohmics_connected_to_voltage_sources =
          std::make_shared<physics::device_structures::Connections>();
  for (const auto &conn : *connections) {
    if (conn->type ==
        physics::device_structures::ConnectionType::VoltageSource) {
      for (const auto &ohmic : conn->connected_ohmics) {
        ohmics_connected_to_voltage_sources->push_back(ohmic);
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
} // namespace falcon::routine
