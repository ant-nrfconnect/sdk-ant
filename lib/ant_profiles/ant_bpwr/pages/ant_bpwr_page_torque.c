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
#include <stdio.h>
#include <ant_profiles/bpwr/pages/ant_bpwr_page_torque.h>
#include <ant_profiles/bpwr/ant_bpwr_utils.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_bpwr_page_torque, CONFIG_BPWR_PAGES_LOG_LEVEL);

typedef struct
{
    uint8_t update_event_count;
    uint8_t tick;
    uint8_t reserved;
    uint8_t period[2];
    uint8_t accumulated_torque[2];
}ant_bpwr_page_torque_data_layout_t;

void ant_bpwr_page_torque_log(ant_bpwr_page_torque_data_t const * p_page_data)
{
    uint16_t period     = ANT_BPWR_TORQUE_PERIOD_RESCALE(p_page_data->period);
    uint32_t acc_torque = ANT_BPWR_ACC_TORQUE_RESCALE(p_page_data->accumulated_torque);

    LOG_INF("event count:                    %u", p_page_data->update_event_count);
    LOG_INF("tick:                           %u", p_page_data->tick);
    LOG_INF("period:                         %u.%03us",
                  (unsigned int)(period / ANT_BPWR_TORQUE_PERIOD_DISP_PRECISION),
                  (unsigned int)(period % ANT_BPWR_TORQUE_PERIOD_DISP_PRECISION));
    LOG_INF("accumulated torque:             %u.%01uNm",
                  (unsigned int)(acc_torque / ANT_BPWR_ACC_TORQUE_DISP_PRECISION),
                  (unsigned int)(acc_torque % ANT_BPWR_ACC_TORQUE_DISP_PRECISION));
}


void ant_bpwr_page_torque_encode(uint8_t                           * p_page_buffer,
                                 ant_bpwr_page_torque_data_t const * p_page_data)
{
    ant_bpwr_page_torque_data_layout_t * p_outcoming_data =
        (ant_bpwr_page_torque_data_layout_t *)p_page_buffer;

    p_outcoming_data->update_event_count    = p_page_data->update_event_count;
    p_outcoming_data->tick                  = p_page_data->tick;

    uint16_encode(p_page_data->period, p_outcoming_data->period);
    uint16_encode(p_page_data->accumulated_torque,
                                   p_outcoming_data->accumulated_torque);
}


void ant_bpwr_page_torque_decode(uint8_t const               * p_page_buffer,
                                 ant_bpwr_page_torque_data_t * p_page_data)
{
    ant_bpwr_page_torque_data_layout_t const * p_incoming_data =
        (ant_bpwr_page_torque_data_layout_t *)p_page_buffer;

    p_page_data->update_event_count    = p_incoming_data->update_event_count;
    p_page_data->tick                  = p_incoming_data->tick;
    p_page_data->period                = uint16_decode(p_incoming_data->period);
    p_page_data->accumulated_torque    = uint16_decode(p_incoming_data->accumulated_torque);
}
