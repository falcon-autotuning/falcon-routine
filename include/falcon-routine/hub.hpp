#pragma once
#include "falcon-routine/export.h"
#include <falcon-core/communications/messages/MeasurementRequest.hpp>
#include <falcon-core/communications/messages/MeasurementResponse.hpp>
#include <falcon-core/communications/messages/VoltageStatesResponse.hpp>
#include <falcon-core/communications/voltage_states/DeviceVoltageStates.hpp>
#include <falcon-core/instrument_interfaces/names/Ports.hpp>
#include <falcon-core/physics/config/core/Config.hpp>
#include <falcon-core/physics/device_structures/Connections.hpp>
#include <falcon-core/physics/device_structures/GateRelations.hpp>

using namespace falcon::routine;
using namespace falcon_core;

/**
 * @brief Allows access to instrument hub to query current device state
 * @param timeout_ms the timeout in milliseconds to wait
 * @return The VoltageStatesResponse if successful
 */
communications::messages::VoltageStatesResponseSP FALCON_ROUTINE_API
request_device_state(int timeout_ms);

/**
 * @brief Allows access to the instrument hub to request a measurement
 * @param req the request of the measurement to perform
 * @param timeout_ms the timeout in milliseconds to wait
 * @return The MeasurementResponse if successful
 */
communications::messages::MeasurementResponseSP FALCON_ROUTINE_API
request_measurement(const communications::messages::MeasurementRequestSP &req,
                    int timeout_ms);

/**
 * @brief Request the device config from the instrument hub.
 * @param timeout_ms the timeout in milliseconds to wait
 * @return The Config if successful
 */
physics::config::core::ConfigSP FALCON_ROUTINE_API
request_config(int timeout_ms);

/**
 * @brief Request the current port payload from the instrument hub.
 * @param timeout_ms the timeout in milliseconds to wait
 * @return The Ports if successful
 */
std::tuple<instrument_interfaces::names::Ports,
           instrument_interfaces::names::Ports>
    FALCON_ROUTINE_API request_port_payload(int timeout_ms);

/**
 * @brief Cache the device voltages for quick access.
 * @param voltages the voltages to cache
 */
void FALCON_ROUTINE_API cache_device_voltages(
    communications::voltage_states::DeviceVoltageStatesSP voltages);

/**
 * @brief Reads the current device voltages. Checks the cache first before
 * requesting from the instrument hub.
 * @param timeout_ms the timeout in milliseconds to wait if a request is needed
 * @return The DeviceVoltageStates if successful
 */
communications::voltage_states::DeviceVoltageStatesSP FALCON_ROUTINE_API
read_device_voltages(int timeout_ms);

/**
 * @brief Gets the ohmics connected to voltage sources. Caches the result for
 * future calls.
 * @param timeout_ms the timeout in milliseconds to wait if a request is needed
 * @return A set of strings representing the ohmics connected to voltage
 * sources.
 */
physics::device_structures::ConnectionsSP FALCON_ROUTINE_API
get_ohmics_connected_to_voltage_sources(int timeout_ms);

// /**
//  * @brief Gets/Computes the Gate Relations for the current device. Caches the
//  * result for future calls.
//  * @param timeout_ms the timeout in milliseconds to wait if a request is
//  needed
//  * @return The GateRelations for the current device.
//  */
// physics::device_structures::GateRelationsSP FALCON_ROUTINE_API
// get_gate_relations(int timeout_ms);
//
// /**
//  * @brief Finds the safe maximum and mimimum voltages for a set of gates.
//  * @param connections the connections to check for safe voltages
//  * @param bounds the voltage bounds to check for safe voltages
//  * @param max_safe_difference the maximum safe difference between adjacent
//  gates
//  * @param timeout_ms the timeout in milliseconds to wait if a request is
//  needed
//  * @return A map of connections to their safe voltage bounds.
//  */
// generic::MapSP<physics::device_structures::Connections, math::Domain>
//     FALCON_ROUTINE_API get_voltage_bounds(
//         physics::device_structures::ConnectionsSP connections,
//         math::DomainSP bounds, double max_safe_difference, int timeout_ms););
//
// /**
//  * @brief Determines if the voltage change is safe.
//  * @param proposed_voltages the proposed voltage changes to check
//  * @param bounds the voltage bounds to check against
//  * @param max_safe_difference the maximum safe difference between adjacent
//  gates
//  * @param timeout_ms the timeout in milliseconds to wait if a request is
//  needed
//  * @return bool indicating if the voltage change is safe or not.
//  */
// bool FALCON_ROUTINE_API safe_voltage_change(math::PointSP proposed_voltages,
//                                             math::DomainSP bounds,
//                                             double max_safe_difference,
//                                             int timeout_ms);
