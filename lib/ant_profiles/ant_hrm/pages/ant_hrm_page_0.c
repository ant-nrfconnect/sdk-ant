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

#include <ant_profiles/hrm/pages/ant_hrm_page_0.h>
#include <ant_profiles/hrm/ant_hrm_utils.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_hrm_page_0, CONFIG_HRM_PAGES_LOG_LEVEL);

/**@brief HRM page 0 data layout structure. */
typedef struct
{
    uint8_t reserved[3];
    uint8_t heart_beat_evt_time_LSB;
    uint8_t heart_beat_evt_time_MSB;
    uint8_t heart_beat_count;
    uint8_t computed_heart_rate;
}ant_hrm_page0_data_layout_t;

/**@brief Function for tracing page 0 and common data.
 *
 * @param[in]  p_common_data    Pointer to the common data.
 * @param[in]  p_page_data      Pointer to the page 0 data.
 */
static void page0_data_log(ant_hrm_page0_data_t const * p_page_data)
{
    LOG_INF("Heart beat count:                 %u", (unsigned int)p_page_data->beat_count);
    LOG_INF("Computed heart rate:              %u",
                 (unsigned int) p_page_data->computed_heart_rate);
    LOG_INF("Heart beat event time:            %u.%03us\n",
                 (unsigned int) ANT_HRM_BEAT_TIME_SEC(p_page_data->beat_time),
                 (unsigned int) ANT_HRM_BEAT_TIME_MSEC(p_page_data->beat_time));
}


void ant_hrm_page_0_encode(uint8_t                    * p_page_buffer,
                           ant_hrm_page0_data_t const * p_page_data)
{
    ant_hrm_page0_data_layout_t * p_outcoming_data = (ant_hrm_page0_data_layout_t *)p_page_buffer;
    uint32_t                      beat_time        = p_page_data->beat_time;

    p_outcoming_data->reserved[0]             = UINT8_MAX;
    p_outcoming_data->reserved[1]             = UINT8_MAX;
    p_outcoming_data->reserved[2]             = UINT8_MAX;
    p_outcoming_data->heart_beat_evt_time_LSB = (uint8_t)(beat_time & UINT8_MAX);
    p_outcoming_data->heart_beat_evt_time_MSB = (uint8_t)((beat_time >> 8) & UINT8_MAX);
    p_outcoming_data->heart_beat_count        = (uint8_t)p_page_data->beat_count;
    p_outcoming_data->computed_heart_rate     = (uint8_t)p_page_data->computed_heart_rate;

    page0_data_log(p_page_data);
}


void ant_hrm_page_0_decode(uint8_t const        * p_page_buffer,
                           ant_hrm_page0_data_t * p_page_data)
{
    ant_hrm_page0_data_layout_t const * p_incoming_data =
        (ant_hrm_page0_data_layout_t *)p_page_buffer;

    uint32_t beat_time = (uint32_t)((p_incoming_data->heart_beat_evt_time_MSB << 8)
                                    + p_incoming_data->heart_beat_evt_time_LSB);

    p_page_data->beat_count          = (uint32_t)p_incoming_data->heart_beat_count;
    p_page_data->computed_heart_rate = (uint32_t)p_incoming_data->computed_heart_rate;
    p_page_data->beat_time           = beat_time;

    page0_data_log(p_page_data);
}
