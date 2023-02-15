/*
 * Copyright (c) 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 *
 * Copyright (c) 2015 - 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef ANT_BPWR_SIMULATOR_LOCAL_H__
#define ANT_BPWR_SIMULATOR_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include <ant_profiles/bpwr/ant_bpwr.h>
#include <sensorsim.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup ant_sdk_bpwr_simulator
 * @brief BPWR simulator control block structure. */
typedef struct
{
    bool              auto_change;             ///< Power will change automatically (if auto_change is set) or manually.
    uint32_t          tick_incr;               ///< Fractional part of tick increment.
    sensorsim_state_t power_sensorsim_state;   ///< Power state of the simulated sensor.
    sensorsim_cfg_t   power_sensorsim_cfg;     ///< Power configuration of the simulated sensor.
    sensorsim_state_t cadence_sensorsim_state; ///< Cadence stated of the simulated sensor.
    sensorsim_cfg_t   cadence_sensorsim_cfg;   ///< Cadence configuration of the simulated sensor.
    sensorsim_state_t pedal_sensorsim_state;   ///< Pedal state of the simulated sensor.
    sensorsim_cfg_t   pedal_sensorsim_cfg;     ///< Pedal configuration of the simulated sensor.
}ant_bpwr_simulator_cb_t;



#ifdef __cplusplus
}
#endif

#endif // ANT_BPWR_SIMULATOR_LOCAL_H__
