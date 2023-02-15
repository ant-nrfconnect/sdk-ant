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

#ifndef ANT_BPWR_LOCAL_H__
#define ANT_BPWR_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include "ant_profiles/bpwr/ant_bpwr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ant_bpwr
 * @{
 */

/** @brief Bicycle Power Sensor control block. */
typedef struct
{
    uint8_t           message_counter;
    ant_bpwr_torque_t torque_use;
    enum
    {
        BPWR_SENS_CALIB_NONE,      ///< Idle state.
        BPWR_SENS_CALIB_REQUESTED, ///< Received request for general calibration result message by the sensor.
        BPWR_SENS_CALIB_READY,     ///< Calibration response message is ready to be transmitted.
    }                        calib_stat;
    ant_bpwr_calib_handler_t calib_handler;
} ant_bpwr_sens_cb_t;

/**@brief Bicycle Power Sensor RX control block. */
typedef struct
{
    uint8_t calib_timeout;
    enum
    {
        BPWR_DISP_CALIB_NONE,      ///< Idle state.
        BPWR_DISP_CALIB_REQUESTED, ///< Calibration requested.
    } calib_stat;
} ant_bpwr_disp_cb_t;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // ANT_BPWR_LOCAL_H__
