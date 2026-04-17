#pragma once
#include "falcon-routine/export.h"
#include <falcon-comms/routine_comms.hpp>
#include <falcon-core/communications/messages/MeasurementRequest.hpp>
#include <falcon-core/communications/messages/MeasurementResponse.hpp>
#include <falcon-core/communications/messages/VoltageStatesResponse.hpp>

namespace falcon::routine {

/**
 * @brief Alias for Routine Comms but exposed within the routine
 * library.
 */
using Comms = falcon::comms::RoutineComms;

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

} // namespace falcon::routine
