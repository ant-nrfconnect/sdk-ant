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
#include <ant_profiles/bsc/pages/ant_bsc_combined_page_0.h>
#include <ant_profiles/bsc/ant_bsc_utils.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_bsc_combined_page_0, CONFIG_BSC_PAGES_LOG_LEVEL);


/**@brief BSC page 0 data layout structure. */
typedef struct
{
    uint8_t cadence_evt_time_LSB;
    uint8_t cadence_evt_time_MSB;
    uint8_t cadence_rev_count_LSB;
    uint8_t cadence_rev_count_MSB;
    uint8_t speed_evt_time_LSB;
    uint8_t speed_evt_time_MSB;
    uint8_t speed_rev_count_LSB;
    uint8_t speed_rev_count_MSB;
}ant_bsc_combined_page0_data_layout_t;

/**@brief Function for printing combined speed and cadence page 0 data. */
static void comb_page0_data_log(ant_bsc_combined_page0_data_t const * p_page_data)
{
    LOG_INF("Cadence revolution count:    %u",
                   (unsigned int)p_page_data->cadence_rev_count);

    LOG_INF("Cadence event time:          %u.%03us",
                   (unsigned int)ANT_BSC_EVENT_TIME_SEC(p_page_data->cadence_event_time),
                   (unsigned int)ANT_BSC_EVENT_TIME_MSEC(p_page_data->cadence_event_time));

    LOG_INF("Speed revolution count:      %u",
                   (unsigned int)p_page_data->speed_rev_count);

    LOG_INF("Speed event time:            %u.%03us\r\n",
                   (unsigned int)ANT_BSC_EVENT_TIME_SEC(p_page_data->speed_event_time),
                   (unsigned int)ANT_BSC_EVENT_TIME_MSEC(p_page_data->speed_event_time));
}

void ant_bsc_combined_page_0_encode(uint8_t * p_page_buffer, ant_bsc_combined_page0_data_t const * p_page_data)
{
    ant_bsc_combined_page0_data_layout_t * p_outcoming_data = (ant_bsc_combined_page0_data_layout_t *) p_page_buffer;

    uint16_t cadence_event_time = p_page_data->cadence_event_time;
    uint16_t cadence_rev_count  = p_page_data->cadence_rev_count;
    uint16_t speed_event_time   = p_page_data->speed_event_time;
    uint16_t speed_rev_count    = p_page_data->speed_rev_count;

    p_outcoming_data->cadence_evt_time_LSB  = (uint8_t)(cadence_event_time & UINT8_MAX);
    p_outcoming_data->cadence_evt_time_MSB  = (uint8_t)((cadence_event_time >> 8) & UINT8_MAX);
    p_outcoming_data->cadence_rev_count_LSB = (uint8_t)(cadence_rev_count & UINT8_MAX);
    p_outcoming_data->cadence_rev_count_MSB = (uint8_t)((cadence_rev_count >> 8) & UINT8_MAX);
    p_outcoming_data->speed_evt_time_LSB    = (uint8_t)(speed_event_time & UINT8_MAX);
    p_outcoming_data->speed_evt_time_MSB    = (uint8_t)((speed_event_time >> 8) & UINT8_MAX);
    p_outcoming_data->speed_rev_count_LSB   = (uint8_t)(speed_rev_count & UINT8_MAX);
    p_outcoming_data->speed_rev_count_MSB   = (uint8_t)((speed_rev_count >> 8) & UINT8_MAX);

    comb_page0_data_log(p_page_data);
}

void ant_bsc_combined_page_0_decode(uint8_t const * p_page_buffer, ant_bsc_combined_page0_data_t * p_page_data)
{
    ant_bsc_combined_page0_data_layout_t const * p_incoming_data = (ant_bsc_combined_page0_data_layout_t *)p_page_buffer;

    uint16_t cadence_event_time = (uint16_t)((p_incoming_data->cadence_evt_time_MSB << 8)
                                  + p_incoming_data->cadence_evt_time_LSB);
    uint16_t cadence_revolution_count = (uint16_t) ((p_incoming_data->cadence_rev_count_MSB << 8)
                                        + p_incoming_data->cadence_rev_count_LSB);
    uint16_t speed_event_time = (uint16_t)((p_incoming_data->speed_evt_time_MSB << 8)
                                + p_incoming_data->speed_evt_time_LSB);
    uint16_t speed_revolution_count = (uint16_t) ((p_incoming_data->speed_rev_count_MSB << 8)
                                      + p_incoming_data->speed_rev_count_LSB);

    p_page_data->cadence_event_time = cadence_event_time;
    p_page_data->cadence_rev_count  = cadence_revolution_count;
    p_page_data->speed_event_time   = speed_event_time;
    p_page_data->speed_rev_count    = speed_revolution_count;

    comb_page0_data_log(p_page_data);
}
