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
#include <ant_profiles/bsc/pages/ant_bsc_page_5.h>
#include <ant_profiles/bsc/ant_bsc_utils.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_bsc_page_5, CONFIG_BSC_PAGES_LOG_LEVEL);

/**@brief BSC profile page 5 bitfields definitions. */
#define ANT_BSC_STOP_IND_MASK       0x01

/**@brief BSC page 5 data layout structure. */
typedef struct
{
    uint8_t flags;
    uint8_t reserved[6];
}ant_bsc_page5_data_layout_t;

/**@brief Function for printing speed or cadence page5 data. */
static void page5_data_log(ant_bsc_page5_data_t const * p_page_data)
{
    if (p_page_data->stop_indicator)
    {
        LOG_INF("Bicycle stopped.\n\n");
    }
    else
    {
        LOG_INF("Bicycle moving.\n\n");
    }
}

void ant_bsc_page_5_encode(uint8_t * p_page_buffer, ant_bsc_page5_data_t const * p_page_data)
{
    ant_bsc_page5_data_layout_t * p_outcoming_data = (ant_bsc_page5_data_layout_t *)p_page_buffer;

    p_outcoming_data->flags = (uint8_t)p_page_data->stop_indicator;

    page5_data_log( p_page_data);
}

void ant_bsc_page_5_decode(uint8_t const * p_page_buffer, ant_bsc_page5_data_t * p_page_data)
{
    ant_bsc_page5_data_layout_t const * p_incoming_data = (ant_bsc_page5_data_layout_t *)p_page_buffer;

    p_page_data->stop_indicator  = (p_incoming_data->flags) & ANT_BSC_STOP_IND_MASK;

    page5_data_log( p_page_data);
}
