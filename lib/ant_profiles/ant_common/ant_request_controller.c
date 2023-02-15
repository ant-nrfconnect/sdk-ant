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

#include <ant_interface.h>
#include <ant_profiles/common/ant_request_controller.h>
#include <ant_parameters.h>

/**@brief Common message data layout structure. */
typedef struct
{
    uint8_t page_number;     ///< Page number.
    uint8_t page_payload[7]; ///< Page payload.
}ant_common_message_layout_t;


void ant_request_controller_init(ant_request_controller_t * p_controller)
{
    __ASSERT_NO_MSG(p_controller != NULL);

    p_controller->state   = ANT_REQUEST_CONTROLLER_IDLE;
    p_controller->page_70 = DEFAULT_ANT_COMMON_PAGE70();
}


uint32_t ant_request_controller_request(ant_request_controller_t * p_controller,
                                        uint8_t                    channel_number,
                                        ant_common_page70_data_t * p_page_70)
{
    __ASSERT_NO_MSG(p_controller != NULL);
    __ASSERT_NO_MSG(p_page_70 != NULL);

    ant_common_message_layout_t message;
    message.page_number = ANT_COMMON_PAGE_70;

    ant_common_page_70_encode(message.page_payload, p_page_70);

    p_controller->state = ANT_REQUEST_CONTROLLER_SENDED;

    return ant_acknowledge_message_tx(channel_number, sizeof (message), (uint8_t *)&message);
}


bool ant_request_controller_pending_get(ant_request_controller_t * p_controller,
                                        uint8_t                  * p_page_number)
{
    __ASSERT_NO_MSG(p_controller != NULL);
    __ASSERT_NO_MSG(p_page_number != NULL);

    if ((p_controller->state == ANT_REQUEST_CONTROLLER_ACK_UNTIL_SUCCESS_REQUESTED)
        || (p_controller->state == ANT_REQUEST_CONTROLLER_ACK_REQUESTED)
        || (p_controller->state == ANT_REQUEST_CONTROLLER_BROADCAST_REQUESTED))
    {
        *p_page_number = p_controller->page_70.page_number;
        return true;
    }
    return false;
}


bool ant_request_controller_ack_needed(ant_request_controller_t * p_controller)
{
    __ASSERT_NO_MSG(p_controller != NULL);
    return ((p_controller->state == ANT_REQUEST_CONTROLLER_ACK_UNTIL_SUCCESS_REQUESTED)
            || (p_controller->state == ANT_REQUEST_CONTROLLER_ACK_REQUESTED));
}


ant_request_controller_evt_t ant_request_controller_disp_evt_handler(
    ant_request_controller_t * p_controller,
    ant_evt_t                * p_ant_evt)
{
    __ASSERT_NO_MSG(p_controller != NULL);
    __ASSERT_NO_MSG(p_ant_evt != NULL);

    if ( p_controller->state == ANT_REQUEST_CONTROLLER_SENDED)
    {
        switch (p_ant_evt->event)
        {
            case EVENT_TRANSFER_TX_FAILED:
                p_controller->state = ANT_REQUEST_CONTROLLER_IDLE;
                return ANT_REQUEST_CONTROLLER_FAILED;

            case EVENT_TRANSFER_TX_COMPLETED:
                p_controller->state = ANT_REQUEST_CONTROLLER_IDLE;
                return ANT_REQUEST_CONTROLLER_SUCCESS;

            default:
                break;
        }
    }

    return ANT_REQUEST_CONTROLLER_NONE;
}


/**@brief Function for confirmation of page sending.
 *
 * @param[in]  p_controller     Pointer to the controller instance.
 */
__STATIC_INLINE void page_sended(ant_request_controller_t * p_controller)
{
    if (!((p_controller->page_70.transmission_response.items.transmit_count)--))
    {
        p_controller->state = ANT_REQUEST_CONTROLLER_IDLE;
    }
}


/**@brief Function for handling page request.
 *
 * @param[in]  p_controller      Pointer to the controller instance.
 * @param[in]  p_message_payload Pointer to the message payload.
 */
__STATIC_INLINE void page_requested(ant_request_controller_t    * p_controller,
                                    ant_common_message_layout_t * p_message_payload)
{
    ant_common_page_70_decode(p_message_payload->page_payload, &(p_controller->page_70));

    if ((p_controller->page_70.command_type == ANT_PAGE70_COMMAND_PAGE_DATA_REQUEST)
        && (p_controller->page_70.transmission_response.specyfic != ANT_PAGE70_RESPONSE_INVALID))
    {
        if (p_controller->page_70.transmission_response.items.ack_resposne)
        {
            if (p_controller->page_70.transmission_response.specyfic ==
                ANT_PAGE70_RESPONSE_TRANSMIT_UNTIL_SUCCESS)
            {
                p_controller->state = ANT_REQUEST_CONTROLLER_ACK_UNTIL_SUCCESS_REQUESTED;
            }
            else
            {
                p_controller->state = ANT_REQUEST_CONTROLLER_ACK_REQUESTED;
            }
        }
        else
        {
            p_controller->state = ANT_REQUEST_CONTROLLER_BROADCAST_REQUESTED;
        }
    }
}


void ant_request_controller_sens_evt_handler(ant_request_controller_t * p_controller,
                                             ant_evt_t                * p_ant_evt)
{
    __ASSERT_NO_MSG(p_controller != NULL);
    __ASSERT_NO_MSG(p_ant_evt != NULL);

    ant_common_message_layout_t * p_message_payload =
        (ant_common_message_layout_t *)p_ant_evt->message.ANT_MESSAGE_aucPayload;

    switch (p_ant_evt->event)
    {
        case EVENT_RX:

            if (p_message_payload->page_number == ANT_COMMON_PAGE_70)
            {
                page_requested(p_controller, p_message_payload);
            }
            break;

        case EVENT_TRANSFER_TX_COMPLETED:

            if (p_controller->state == ANT_REQUEST_CONTROLLER_ACK_UNTIL_SUCCESS_REQUESTED)
            {
                p_controller->state = ANT_REQUEST_CONTROLLER_IDLE;
            }
            /* fall through */

        case EVENT_TX:

            if (p_controller->state == ANT_REQUEST_CONTROLLER_BROADCAST_REQUESTED
                || p_controller->state == ANT_REQUEST_CONTROLLER_ACK_REQUESTED)
            {
                page_sended(p_controller);
            }
            break;

        default:
            break;
    }
}

