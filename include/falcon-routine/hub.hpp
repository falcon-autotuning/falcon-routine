#pragma once
#include "falcon-routine/export.h"
#include <falcon-core/communications/messages/MeasurementRequest.hpp>
#include <falcon-core/communications/messages/MeasurementResponse.hpp>
#include <falcon-core/communications/messages/VoltageStatesResponse.hpp>
#include <falcon-core/instrument_interfaces/names/Ports.hpp>
#include <falcon-core/physics/config/core/Config.hpp>

namespace falcon::routine {

/**
 * @brief Allows access to instrument hub to query current device state
 * @param timeout_ms the timeout in milliseconds to wait
 * @return The VoltageStatesResponse if successful
 */
falcon_core::communications::messages::VoltageStatesResponseSP
    FALCON_ROUTINE_API
    request_device_state(int timeout_ms);

/**
 * @brief Allows access to the instrument hub to request a measurement
 * @param req the request of the measurement to perform
 * @param timeout_ms the timeout in milliseconds to wait
 * @return The MeasurementResponse if successful
 */
falcon_core::communications::messages::MeasurementResponseSP FALCON_ROUTINE_API
request_measurement(
    const falcon_core::communications::messages::MeasurementRequestSP &req,
    int timeout_ms);

/**
 * @brief Request the device config from the instrument hub.
 * @param timeout_ms the timeout in milliseconds to wait
 * @return The Config if successful
 */
falcon_core::physics::config::core::ConfigSP FALCON_ROUTINE_API
request_config(int timeout_ms);

/**
 * @brief Request the current port payload from the instrument hub.
 * @param timeout_ms the timeout in milliseconds to wait
 * @return The Ports if successful
 */
std::tuple<falcon_core::instrument_interfaces::names::Ports,
           falcon_core::instrument_interfaces::names::Ports>
    FALCON_ROUTINE_API request_port_payload(int timeout_ms);
} // namespace falcon::routine
