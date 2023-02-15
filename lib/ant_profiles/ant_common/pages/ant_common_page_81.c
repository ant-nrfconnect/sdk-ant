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
#include <ant_profiles/common/pages/ant_common_page_81.h>
#include <ant_profiles/bpwr/ant_bpwr_utils.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ant_common_page_81, LOG_LEVEL_INF);

/**@brief ant+ common page 81 data layout structure. */
typedef struct
{
    uint8_t reserved; ///< unused, fill by 0xFF
    uint8_t sw_revision_minor;
    uint8_t sw_revision_major;
    uint8_t serial_number[4];
}ant_common_page81_data_layout_t;


/**@brief Function for tracing page 80 data.
 *
 * @param[in]  p_page_data      Pointer to the page 80 data.
 */
static void page81_data_log(volatile ant_common_page81_data_t const * p_page_data)
{
    if (p_page_data->sw_revision_minor != UINT8_MAX)
    {
        LOG_INF("sw revision:                      %u.%u",
                   ((ant_common_page81_data_t const *) p_page_data)->sw_revision_major,
                   ((ant_common_page81_data_t const *) p_page_data)->sw_revision_minor);
    }
    else
    {
        LOG_INF("sw revision:                      %u", p_page_data->sw_revision_major);
    }

    LOG_INF("serial number:                    %u\r\n\n", (unsigned int) p_page_data->serial_number);
}


void ant_common_page_81_encode(uint8_t                                 * p_page_buffer,
                               volatile ant_common_page81_data_t const * p_page_data)
{
    ant_common_page81_data_layout_t * p_outcoming_data =
        (ant_common_page81_data_layout_t *)p_page_buffer;

    p_outcoming_data->reserved = UINT8_MAX;

    p_outcoming_data->sw_revision_minor = p_page_data->sw_revision_minor;
    p_outcoming_data->sw_revision_major = p_page_data->sw_revision_major;

    uint32_encode(p_page_data->serial_number, p_outcoming_data->serial_number);

    page81_data_log(p_page_data);
}


void ant_common_page_81_decode(uint8_t const                     * p_page_buffer,
                               volatile ant_common_page81_data_t * p_page_data)
{
    ant_common_page81_data_layout_t const * p_incoming_data =
        (ant_common_page81_data_layout_t *)p_page_buffer;

    p_page_data->sw_revision_minor = p_incoming_data->sw_revision_minor;
    p_page_data->sw_revision_major = p_incoming_data->sw_revision_major;

    p_page_data->serial_number = uint32_decode(p_incoming_data->serial_number);

    page81_data_log(p_page_data);
}
