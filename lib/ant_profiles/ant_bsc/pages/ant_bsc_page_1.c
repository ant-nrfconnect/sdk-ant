/*
 * Copyright (c) 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 *
 * Copyright (c) 2015 - 2021, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <ant_profiles/bsc/pages/ant_bsc_page_1.h>
#include <ant_profiles/bsc/ant_bsc_utils.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_bsc_page_1, CONFIG_BSC_PAGES_LOG_LEVEL);

/**@brief BSC page 1 data layout structure. */
typedef struct
{
    uint8_t cumulative_operating_time[3];
    uint8_t reserved[4];
}ant_bsc_page1_data_layout_t;

/**@brief Function for printing speed or cadence page1 data. */
static void page1_data_log(ant_bsc_page1_data_t const * p_page_data)
{
    LOG_INF("Cumulative operating time:    %ud %uh %um %us\n",
              (unsigned int)ANT_BSC_OPERATING_DAYS(p_page_data->operating_time),
              (unsigned int)ANT_BSC_OPERATING_HOURS(p_page_data->operating_time),
              (unsigned int)ANT_BSC_OPERATING_MINUTES(p_page_data->operating_time),
              (unsigned int)ANT_BSC_OPERATING_SECONDS(p_page_data->operating_time));
}

void ant_bsc_page_1_encode(uint8_t * p_page_buffer, ant_bsc_page1_data_t const * p_page_data)
{
    ant_bsc_page1_data_layout_t * p_outcoming_data = (ant_bsc_page1_data_layout_t *)p_page_buffer;
    uint32_t                      operating_time   = p_page_data->operating_time;
    uint8_t * p_encoded_data                       = p_outcoming_data->cumulative_operating_time;

    p_encoded_data[0] = (uint8_t) ((operating_time & 0x000000FF) >> 0);
    p_encoded_data[1] = (uint8_t) ((operating_time & 0x0000FF00) >> 8);
    p_encoded_data[2] = (uint8_t) ((operating_time & 0x00FF0000) >> 16);

    page1_data_log( p_page_data);
}

void ant_bsc_page_1_decode(uint8_t const * p_page_buffer, ant_bsc_page1_data_t * p_page_data)
{
    ant_bsc_page1_data_layout_t const * p_incoming_data = (ant_bsc_page1_data_layout_t *)p_page_buffer;

    const uint8_t * p_encoded_data = p_incoming_data->cumulative_operating_time;

    uint32_t operating_time = ((((uint32_t)((uint8_t *)p_encoded_data)[0]) << 0)  |
                               (((uint32_t)((uint8_t *)p_encoded_data)[1]) << 8)  |
                               (((uint32_t)((uint8_t *)p_encoded_data)[2]) << 16));

    p_page_data->operating_time = operating_time;

    page1_data_log( p_page_data);
}
