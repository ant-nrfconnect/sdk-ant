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
#include <string.h>
#include <ant_profiles/common/pages/ant_common_page_80.h>
#include <ant_profiles/common/pages/ant_common_page_81.h>
#include <ant_profiles/bpwr/ant_bpwr_utils.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ant_common_page_80, LOG_LEVEL_INF);

/**@brief ant+ common page 80 data layout structure. */
typedef struct
{
    uint8_t reserved[2]; ///< unused, fill by 0xFF
    uint8_t hw_revision;
    uint8_t manufacturer_id[2];
    uint8_t model_number[2];
}ant_common_page80_data_layout_t;


/**@brief Function for tracing page 80 data.
 *
 * @param[in]  p_page_data      Pointer to the page 80 data.
 */
static void page80_data_log(volatile ant_common_page80_data_t const * p_page_data)
{
    LOG_INF("hw revision:                      %u", p_page_data->hw_revision);
    LOG_INF("manufacturer id:                  %u", p_page_data->manufacturer_id);
    LOG_INF("model number:                     %u\r\n\n", p_page_data->model_number);
}


void ant_common_page_80_encode(uint8_t                                 * p_page_buffer,
                               volatile ant_common_page80_data_t const * p_page_data)
{
    ant_common_page80_data_layout_t * p_outcoming_data =
        (ant_common_page80_data_layout_t *)p_page_buffer;

    memset(p_page_buffer, UINT8_MAX, sizeof (p_outcoming_data->reserved));

    p_outcoming_data->hw_revision = p_page_data->hw_revision;

    uint16_encode(p_page_data->manufacturer_id,
                                   p_outcoming_data->manufacturer_id);
    uint16_encode(p_page_data->model_number, p_outcoming_data->model_number);

    page80_data_log(p_page_data);
}


void ant_common_page_80_decode(uint8_t const                     * p_page_buffer,
                               volatile ant_common_page80_data_t * p_page_data)
{
    ant_common_page80_data_layout_t const * p_incoming_data =
        (ant_common_page80_data_layout_t *)p_page_buffer;

    p_page_data->hw_revision = p_incoming_data->hw_revision;

    p_page_data->manufacturer_id = uint16_decode(p_incoming_data->manufacturer_id);
    p_page_data->model_number    = uint16_decode(p_incoming_data->model_number);

    page80_data_log(p_page_data);
}

