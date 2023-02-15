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
#include <ant_profiles/bsc/pages/ant_bsc_page_4.h>
#include <ant_profiles/bsc/ant_bsc_utils.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_bsc_page_4, CONFIG_BSC_PAGES_LOG_LEVEL);


/**@brief BSC page 4 data layout structure. */
typedef struct
{
    uint8_t reserved_byte;
    uint8_t fract_bat_volt;
    uint8_t coarse_bat_volt   : 4;
    uint8_t bat_status        : 3;
    uint8_t bitfield_reserved : 1;
    uint8_t reserved[4];
}ant_bsc_page4_data_layout_t;

/**@brief Function for printing speed or cadence page4 data. */
static void page4_data_log( ant_bsc_page4_data_t const * p_page_data)
{
    LOG_INF("Battery voltage:              %u.%03uV",
              (unsigned int)p_page_data->coarse_bat_volt,
              (unsigned int)ANT_BSC_BAT_VOLTAGE_FRACTION_MV(p_page_data->fract_bat_volt));
    LOG_INF("Battery status:               %u\n", (unsigned int)p_page_data->bat_status);
}

void ant_bsc_page_4_encode(uint8_t * p_page_buffer, ant_bsc_page4_data_t const * p_page_data)
{
    ant_bsc_page4_data_layout_t * p_outcoming_data = (ant_bsc_page4_data_layout_t *)p_page_buffer;

    p_outcoming_data->reserved_byte     = UINT8_MAX;
    p_outcoming_data->fract_bat_volt    = p_page_data->fract_bat_volt;
    p_outcoming_data->coarse_bat_volt   = p_page_data->coarse_bat_volt;
    p_outcoming_data->bat_status        = p_page_data->bat_status;
    p_outcoming_data->bitfield_reserved = 0;

    page4_data_log( p_page_data);
}

void ant_bsc_page_4_decode(uint8_t const * p_page_buffer, ant_bsc_page4_data_t * p_page_data)
{
    ant_bsc_page4_data_layout_t const * p_incoming_data = (ant_bsc_page4_data_layout_t *)p_page_buffer;

    p_page_data->fract_bat_volt  = p_incoming_data->fract_bat_volt;
    p_page_data->coarse_bat_volt = p_incoming_data->coarse_bat_volt;
    p_page_data->bat_status      = p_incoming_data->bat_status;

    page4_data_log( p_page_data);
}

