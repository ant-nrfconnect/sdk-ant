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
#include <ant_profiles/bpwr/pages/ant_bpwr_page_18.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_bpwr_page_18, CONFIG_BPWR_PAGES_LOG_LEVEL);

static void page18_data_log(ant_bpwr_page18_data_t const * p_page_data)
{
    LOG_INF("                                    Crank Torque");
    ant_bpwr_page_torque_log((ant_bpwr_page_torque_data_t *) p_page_data);
}


void ant_bpwr_page_18_encode(uint8_t                      * p_page_buffer,
                             ant_bpwr_page18_data_t const * p_page_data)
{
    ant_bpwr_page_torque_encode(p_page_buffer, (ant_bpwr_page_torque_data_t *)p_page_data);
    page18_data_log(p_page_data);
}


void ant_bpwr_page_18_decode(uint8_t const          * p_page_buffer,
                             ant_bpwr_page18_data_t * p_page_data)
{
    ant_bpwr_page_torque_decode(p_page_buffer, (ant_bpwr_page_torque_data_t *) p_page_data);
    page18_data_log(p_page_data);
}

