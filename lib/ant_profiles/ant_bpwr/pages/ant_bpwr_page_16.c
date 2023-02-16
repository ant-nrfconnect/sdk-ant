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
#include <ant_profiles/bpwr/pages/ant_bpwr_page_16.h>
#include <ant_profiles/bpwr/ant_bpwr_utils.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_bpwr_page_16, CONFIG_BPWR_PAGES_LOG_LEVEL);

/**@brief bicycle power page 16 data layout structure. */
typedef struct
{
    uint8_t update_event_count;
    uint8_t pedal_power;
    uint8_t reserved;
    uint8_t accumulated_power[2];
    uint8_t instantaneous_power[2];
}ant_bpwr_page16_data_layout_t;


static void page16_data_log(ant_bpwr_page16_data_t const * p_page_data)
{
    LOG_INF("event count:                        %u", p_page_data->update_event_count);

    if (p_page_data->pedal_power.byte != 0xFF)
    {
        LOG_INF("pedal power:                        %u %%",
                   p_page_data->pedal_power.items.distribution);
    }
    else
    {
        LOG_INF("pedal power:                        --");
    }

    LOG_INF("accumulated power:                  %u W", p_page_data->accumulated_power);
    LOG_INF("instantaneous power:                %u W", p_page_data->instantaneous_power);
}


void ant_bpwr_page_16_encode(uint8_t                      * p_page_buffer,
                             ant_bpwr_page16_data_t const * p_page_data)
{
    ant_bpwr_page16_data_layout_t * p_outcoming_data =
        (ant_bpwr_page16_data_layout_t *)p_page_buffer;

    p_outcoming_data->update_event_count    = p_page_data->update_event_count;
    p_outcoming_data->pedal_power           = p_page_data->pedal_power.byte;

    uint16_encode(p_page_data->accumulated_power,
                                   p_outcoming_data->accumulated_power);
    uint16_encode(p_page_data->instantaneous_power,
                                   p_outcoming_data->instantaneous_power);

    page16_data_log(p_page_data);
}


void ant_bpwr_page_16_decode(uint8_t const          * p_page_buffer,
                             ant_bpwr_page16_data_t * p_page_data)
{
    ant_bpwr_page16_data_layout_t const * p_incoming_data =
        (ant_bpwr_page16_data_layout_t *)p_page_buffer;

    p_page_data->update_event_count    = p_incoming_data->update_event_count;
    p_page_data->pedal_power.byte      = p_incoming_data->pedal_power;
    p_page_data->accumulated_power     = uint16_decode(p_incoming_data->accumulated_power);
    p_page_data->instantaneous_power   = uint16_decode(p_incoming_data->instantaneous_power);

    page16_data_log(p_page_data);
}
