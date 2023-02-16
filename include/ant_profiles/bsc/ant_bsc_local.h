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
#ifndef ANT_BSC_LOCAL_H__
#define ANT_BSC_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include "ant_profiles/bsc/ant_bsc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ant_bsc
 * @{
 */

/**@brief BSC Sensor control block. */
typedef struct
{
    uint8_t         device_type;
    uint8_t         toggle_bit          : 1;
    ant_bsc_page_t  main_page_number    : 7;
    uint8_t         page_1_present      : 1;
    uint8_t         page_4_present      : 1;
    ant_bsc_page_t  bkgd_page_number    : 6;
    uint8_t         message_counter;
}ant_bsc_sens_cb_t;

/**@brief BSC Display control block. */
typedef struct
{
    uint8_t         device_type;
}ant_bsc_disp_cb_t;

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif // ANT_BSC_LOCAL_H__
