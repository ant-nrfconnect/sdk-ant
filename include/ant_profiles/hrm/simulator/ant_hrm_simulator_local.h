/*
 * Copyright (c) 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 *
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef ANT_HRM_SIMULATOR_LOCAL_H__
#define ANT_HRM_SIMULATOR_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include <ant_profiles/hrm/ant_hrm.h>
#include <sensorsim.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup ant_sdk_hrm_simulator
 * @brief HRM simulator control block structure. */
typedef struct
{
    bool              auto_change;            ///< Cadence will change automatically (if auto_change is set) or manually.
    uint32_t          time_since_last_hb;     ///< Time since last heart beat occurred (integer part).
    uint64_t          fraction_since_last_hb; ///< Time since last heart beat occurred (fractional part).
    sensorsim_state_t sensorsim_state;        ///< State of the simulated sensor.
    sensorsim_cfg_t   sensorsim_cfg;          ///< Configuration of the simulated sensor.
} ant_hrm_simulator_cb_t;


#ifdef __cplusplus
}
#endif

#endif // ANT_HRM_SIMULATOR_LOCAL_H__
