/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef ANT_HRM_LOCAL_H__
#define ANT_HRM_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include "ant_profiles/ant_hrm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ant_hrm
 * @{
 */

/** @brief HRM Sensor control block. */
typedef struct
{
    uint8_t        toggle_bit;
    ant_hrm_page_t main_page_number;
    uint8_t        page_1_present;
    ant_hrm_page_t ext_page_number;
    uint8_t        message_counter;
} ant_hrm_sens_cb_t;

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif // ANT_HRM_LOCAL_H__
