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
#include <ant_profiles/common/pages/ant_common_page_70.h>
#include <ant_profiles/bpwr/ant_bpwr_utils.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ant_common_page_70, LOG_LEVEL_INF);

/**@brief ANT+ common page 70 data layout structure. */
typedef struct
{
    uint8_t reserved[2];
    uint8_t descriptor[2];
    uint8_t req_trans_response;
    uint8_t req_page_number;
    uint8_t command_type;
}ant_common_page70_data_layout_t;

/**@brief Function for tracing page 70 data.
 *
 * @param[in]  p_page_data      Pointer to the page 70 data.
 */
static void page70_data_log(volatile ant_common_page70_data_t const * p_page_data)
{
    LOG_INF("Page %d request", p_page_data->page_number);

    switch (p_page_data->transmission_response.specyfic)
    {
        case ANT_PAGE70_RESPONSE_TRANSMIT_UNTIL_SUCCESS:
            LOG_INF("Try to send until ACK");
            break;

        case ANT_PAGE70_RESPONSE_INVALID:
            LOG_INF("Invalid requested transmission response");
            break;

        default:

            if (p_page_data->transmission_response.items.ack_resposne)
            {
                LOG_INF("Answer with acknowledged messages");
            }
            LOG_INF("Requested number of transmissions: %d",
                       p_page_data->transmission_response.items.transmit_count);
    }

    switch (p_page_data->command_type)
    {
        case ANT_PAGE70_COMMAND_PAGE_DATA_REQUEST:
            LOG_INF("Request Data Page");
            break;

        case ANT_PAGE70_COMMAND_ANT_FS_SESSION_REQUEST:
            LOG_INF("Request ANT-FS Session");
            break;

        default:
            LOG_INF("Invalid request");
    }
    LOG_INF("Descriptor %x\r\n\n", p_page_data->descriptor);
}


void ant_common_page_70_encode(uint8_t                                 * p_page_buffer,
                               volatile ant_common_page70_data_t const * p_page_data)
{
    ant_common_page70_data_layout_t * p_outcoming_data =
        (ant_common_page70_data_layout_t *)p_page_buffer;

    memset(p_outcoming_data->reserved, UINT8_MAX, sizeof (p_outcoming_data->reserved));
    uint16_encode(p_page_data->descriptor, p_outcoming_data->descriptor);
    p_outcoming_data->req_trans_response = p_page_data->transmission_response.byte;
    p_outcoming_data->req_page_number    = p_page_data->page_number;
    p_outcoming_data->command_type       = p_page_data->command_type;

    page70_data_log(p_page_data);
}


void ant_common_page_70_decode(uint8_t const                     * p_page_buffer,
                               volatile ant_common_page70_data_t * p_page_data)
{
    ant_common_page70_data_layout_t const * p_incoming_data =
        (ant_common_page70_data_layout_t *)p_page_buffer;

    p_page_data->descriptor                 = uint16_decode(p_incoming_data->descriptor);
    p_page_data->transmission_response.byte = p_incoming_data->req_trans_response;
    p_page_data->page_number                = p_incoming_data->req_page_number;
    p_page_data->command_type               = (ant_page70_command_t)p_incoming_data->command_type;

    page70_data_log(p_page_data);
}

