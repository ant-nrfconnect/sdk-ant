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
#include <ant_profiles/bsc/pages/ant_bsc_page_3.h>
#include <ant_profiles/bsc/ant_bsc_utils.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_bsc_page_3, CONFIG_BSC_PAGES_LOG_LEVEL);


/**@brief BSC page 3 data layout structure. */
typedef struct
{
    uint8_t hw_version;
    uint8_t sw_version;
    uint8_t model_num;
    uint8_t reserved[4];
}ant_bsc_page3_data_layout_t;

/**@brief Function for printing speed or cadence page3 data. */
static void page3_data_log(ant_bsc_page3_data_t const * p_page_data)
{
    LOG_INF("Hardware Rev ID:              %u", (unsigned int)p_page_data->hw_version);
    LOG_INF("Model Number:                 %u", (unsigned int)p_page_data->model_num);
    LOG_INF("Software Ver ID:              %u\n", (unsigned int)p_page_data->sw_version);
}

void ant_bsc_page_3_encode(uint8_t * p_page_buffer, ant_bsc_page3_data_t const * p_page_data)
{
    ant_bsc_page3_data_layout_t * p_outcoming_data = (ant_bsc_page3_data_layout_t *)p_page_buffer;

    p_outcoming_data->hw_version = (uint8_t)p_page_data->hw_version;
    p_outcoming_data->sw_version = (uint8_t)p_page_data->sw_version;
    p_outcoming_data->model_num  = (uint8_t)p_page_data->model_num;

    page3_data_log( p_page_data);
}

void ant_bsc_page_3_decode(uint8_t const * p_page_buffer, ant_bsc_page3_data_t * p_page_data)
{
    ant_bsc_page3_data_layout_t const * p_incoming_data = (ant_bsc_page3_data_layout_t *)p_page_buffer;

    p_page_data->hw_version          = (uint32_t)p_incoming_data->hw_version;
    p_page_data->sw_version          = (uint32_t)p_incoming_data->sw_version;
    p_page_data->model_num           = (uint32_t)p_incoming_data->model_num;

    page3_data_log( p_page_data);
}
