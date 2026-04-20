#include "falcon-routine/hub.hpp"
#include <falcon-comms/routine_comms.hpp>
#include <falcon-comms/runtime_comms.hpp>
#include <falcon-core/communications/Time.hpp>
#include <falcon-core/communications/messages/VoltageStatesResponse.hpp>
#include <limits>
#include <stdexcept>

namespace falcon::routine {
using falcon_core::communications::Time;

// TODO: make subscribe_state_response take a long long
falcon_core::communications::messages::VoltageStatesResponseSP
request_device_state(int timeout_ms) {
  falcon::comms::RoutineComms comms;
  long long value = Time().time();
  if (value >= std::numeric_limits<int>::min() &&
      value <= std::numeric_limits<int>::max()) {
    int result = static_cast<int>(value);
    auto resp = comms.subscribe_state_response(timeout_ms, result);
    return falcon_core::communications::messages::VoltageStatesResponse::
        from_json_string<
            falcon_core::communications::messages::VoltageStatesResponse>(
            resp.response);
  }
  throw std::runtime_error("Time is larger than an int");
}

// TODO: make subscribe_measure_response take a long long
falcon_core::communications::messages::MeasurementResponseSP
request_measurement(
    const falcon_core::communications::messages::MeasurementRequestSP &req,
    int timeout_ms) {
  falcon::comms::RoutineComms comms;
  std::string json_req = req->to_json_string();
  long long value = Time().time();
  if (value >= std::numeric_limits<int>::min() &&
      value <= std::numeric_limits<int>::max()) {
    int result = static_cast<int>(value);
    auto resp = comms.subscribe_measure_response(json_req, timeout_ms, result);
    auto outs = comms.pull_measurement_data(resp.stream, resp.channel, 1);
    return falcon_core::communications::messages::MeasurementResponse::
        from_json_string<
            falcon_core::communications::messages::MeasurementResponse>(
            outs[0]);
  }
  throw std::runtime_error("Time is larger than an int");
}

// TODO: make subscribe_measure_response take a long long
falcon_core::physics::config::core::ConfigSP FALCON_ROUTINE_API
request_config(int timeout_ms) {
  falcon::comms::RuntimeComms comms;
  long long value = Time().time();
  if (value >= std::numeric_limits<int>::min() &&
      value <= std::numeric_limits<int>::max()) {
    int result = static_cast<int>(value);
    auto resp = comms.subscribe_config_response(timeout_ms, result);
    return falcon_core::physics::config::core::Config::from_json_string<
        falcon_core::physics::config::core::Config>(resp.response);
  }
  throw std::runtime_error("Time is larger than an int");
}

// TODO: make subscribe_measure_response take a long long
std::tuple<falcon_core::instrument_interfaces::names::Ports,
           falcon_core::instrument_interfaces::names::Ports>
    FALCON_ROUTINE_API request_port_payload(int timeout_ms) {
  falcon::comms::RuntimeComms comms;
  long long value = Time().time();
  if (value >= std::numeric_limits<int>::min() &&
      value <= std::numeric_limits<int>::max()) {
    int result = static_cast<int>(value);
    auto resp = comms.subscribe_port_payload(timeout_ms, result);
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
  throw std::runtime_error("Time is larger than an int");
}
} // namespace falcon::routine
