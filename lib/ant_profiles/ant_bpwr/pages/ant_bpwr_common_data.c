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

#include <ant_profiles/bpwr/pages/ant_bpwr_common_data.h>
#include <ant_profiles/bpwr/ant_bpwr_utils.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_bpwr_common_page, CONFIG_BPWR_PAGES_LOG_LEVEL);

/**@brief BPWR common page data layout structure. */
typedef struct
{
    uint8_t reserved0[2];
    uint8_t instantaneous_cadence;
    uint8_t reserved1[4];
}ant_bpwr_cadence_data_layout_t;

/**@brief Function for tracing common data.
 *
 * @param[in]  p_common_data      Pointer to the common data.
 */
static void cadence_data_log(ant_bpwr_common_data_t const * p_common_data)
{
    if (p_common_data->instantaneous_cadence == 0xFF)
    {
        LOG_INF("instantaneous cadence:          -- rpm\r\n\n");
    }
    else
    {
        LOG_INF("instantaneous cadence:          %u rpm\r\n\n",
                    p_common_data->instantaneous_cadence);
    }
}

void ant_bpwr_cadence_encode(uint8_t                     * p_page_buffer,
                            ant_bpwr_common_data_t const * p_common_data)
{
    ant_bpwr_cadence_data_layout_t * p_outcoming_data = (ant_bpwr_cadence_data_layout_t *)p_page_buffer;
    p_outcoming_data->instantaneous_cadence           = p_common_data->instantaneous_cadence;

    cadence_data_log(p_common_data);
}

void ant_bpwr_cadence_decode(uint8_t const         * p_page_buffer,
                            ant_bpwr_common_data_t * p_common_data)
{
    ant_bpwr_cadence_data_layout_t const * p_incoming_data = (ant_bpwr_cadence_data_layout_t *)p_page_buffer;
    p_common_data->instantaneous_cadence                   = p_incoming_data->instantaneous_cadence;

    cadence_data_log(p_common_data);
}
