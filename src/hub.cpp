#include "falcon-routine/hub.hpp"
#include <falcon-core/communications/Time.hpp>
#include <falcon-core/communications/messages/VoltageStatesResponse.hpp>
#include <limits>
#include <stdexcept>

namespace falcon::routine {
using falcon_core::communications::Time;

// TODO: make subscribe_state_response take a long long
falcon_core::communications::messages::VoltageStatesResponseSP
request_device_state(int timeout_ms) {
  Comms comms;
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
  Comms comms;
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
  throw std::runtime_error("Tiem is larger than an int");
}
} // namespace falcon::routine
