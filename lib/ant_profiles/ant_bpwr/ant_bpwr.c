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
#include <ant_error.h>
#include <ant_interface.h>
#include <ant_profiles/bpwr/ant_bpwr.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_bpwr, CONFIG_ANT_BPWR_LOG_LEVEL);


#define BPWR_CALIB_INT_TIMEOUT ((ANT_CLOCK_FREQUENCY * BPWR_CALIBRATION_TIMOUT_S) / BPWR_MSG_PERIOD) // calibration timeout in ant message period's unit

// for torque sensor Minimum: Interleave every 9th message
#define BPWR_PAGE_16_INTERVAL      5   // Preferred: Interleave every 5th message
#define BPWR_PAGE_16_INTERVAL_OFS  2   // Permissible offset
#define COMMON_PAGE_80_INTERVAL    119 // Minimum: Interleave every 121 messages
#define COMMON_PAGE_81_INTERVAL    120 // Minimum: Interleave every 121 messages
#define AUTO_ZERO_SUPPORT_INTERVAL 120 // Minimum: Interleave every 121 messages

/**@brief Bicycle power message data layout structure. */
typedef struct
{
    uint8_t page_number;
    uint8_t page_payload[7];
} ant_bpwr_message_layout_t;


/**@brief Function for initializing the ANT Bicycle Power Profile instance.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 * @param[in]  p_channel_config Pointer to the ANT channel configuration structure.
 *
 * @retval     0 If initialization was successful. Otherwise, an error code is returned.
 */
static int ant_bpwr_init(ant_bpwr_profile_t         * p_profile,
                                ant_channel_config_t const * p_channel_config)
{
    p_profile->channel_number = p_channel_config->channel_number;

    p_profile->page_1  = DEFAULT_ANT_BPWR_PAGE1();
    p_profile->page_16 = DEFAULT_ANT_BPWR_PAGE16();
    p_profile->page_17 = DEFAULT_ANT_BPWR_PAGE17();
    p_profile->page_18 = DEFAULT_ANT_BPWR_PAGE18();
    p_profile->page_80 = DEFAULT_ANT_COMMON_page80();
    p_profile->page_81 = DEFAULT_ANT_COMMON_page81();

    LOG_INF("ANT B-PWR channel %u init", p_profile->channel_number);
    return ant_channel_init(p_channel_config);
}


int ant_bpwr_disp_init(ant_bpwr_profile_t           * p_profile,
                              ant_channel_config_t const   * p_channel_config,
                              ant_bpwr_disp_config_t const * p_disp_config)
{
    __ASSERT_NO_MSG(p_profile != NULL);
    __ASSERT_NO_MSG(p_channel_config != NULL);
    __ASSERT_NO_MSG(p_disp_config != NULL);
    __ASSERT_NO_MSG(p_disp_config->evt_handler != NULL);
    __ASSERT_NO_MSG(p_disp_config->p_cb != NULL);

    p_profile->evt_handler   = p_disp_config->evt_handler;
    p_profile->_cb.p_disp_cb = p_disp_config->p_cb;

    p_profile->_cb.p_disp_cb ->calib_timeout = 0;
    p_profile->_cb.p_disp_cb ->calib_stat    = BPWR_DISP_CALIB_NONE;

    return ant_bpwr_init(p_profile, p_channel_config);
}


int ant_bpwr_sens_init(ant_bpwr_profile_t           * p_profile,
                              ant_channel_config_t const   * p_channel_config,
                              ant_bpwr_sens_config_t const * p_sens_config)
{
    __ASSERT_NO_MSG(p_profile != NULL);
    __ASSERT_NO_MSG(p_channel_config != NULL);
    __ASSERT_NO_MSG(p_sens_config != NULL);
    __ASSERT_NO_MSG(p_sens_config->p_cb != NULL);
    __ASSERT_NO_MSG(p_sens_config->evt_handler != NULL);
    __ASSERT_NO_MSG(p_sens_config->calib_handler != NULL);

    p_profile->evt_handler   = p_sens_config->evt_handler;
    p_profile->_cb.p_sens_cb = p_sens_config->p_cb;

    p_profile->_cb.p_sens_cb->torque_use      = p_sens_config->torque_use;
    p_profile->_cb.p_sens_cb->calib_handler   = p_sens_config->calib_handler;
    p_profile->_cb.p_sens_cb->calib_stat      = BPWR_SENS_CALIB_NONE;
    p_profile->_cb.p_sens_cb->message_counter = 0;

    return ant_bpwr_init(p_profile, p_channel_config);
}



/**@brief Function for getting next page number to send.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 *
 * @return     Next page number.
 */
static ant_bpwr_page_t next_page_number_get(ant_bpwr_profile_t * p_profile)
{
    ant_bpwr_sens_cb_t * p_bpwr_cb = p_profile->_cb.p_sens_cb;
    ant_bpwr_page_t      page_number;

    if (p_bpwr_cb->calib_stat == BPWR_SENS_CALIB_READY)
    {
        page_number           = ANT_BPWR_PAGE_1;
        p_bpwr_cb->calib_stat = BPWR_SENS_CALIB_NONE; // mark event as processed
    }
    else if ((p_profile->BPWR_PROFILE_auto_zero_status != ANT_BPWR_AUTO_ZERO_NOT_SUPPORTED)
             && (p_bpwr_cb->message_counter == AUTO_ZERO_SUPPORT_INTERVAL))
    {
        page_number                            = ANT_BPWR_PAGE_1;
        p_profile->BPWR_PROFILE_calibration_id = ANT_BPWR_CALIB_ID_AUTO_SUPPORT;
        p_bpwr_cb->message_counter++;
    }
    else if (p_bpwr_cb->message_counter >= COMMON_PAGE_81_INTERVAL)
    {
        page_number                = ANT_BPWR_PAGE_81;
        p_bpwr_cb->message_counter = 0;
    }
    else
    {
        if (p_bpwr_cb->message_counter == COMMON_PAGE_80_INTERVAL)
        {
            page_number = ANT_BPWR_PAGE_80;
        }
        else
        {
            if ( p_bpwr_cb->torque_use == TORQUE_NONE)
            {
                page_number = ANT_BPWR_PAGE_16;
            }
            else if ((p_bpwr_cb->message_counter % BPWR_PAGE_16_INTERVAL)
                     == BPWR_PAGE_16_INTERVAL_OFS)
            {
                page_number = ANT_BPWR_PAGE_16;
            }
            else if ( p_bpwr_cb->torque_use == TORQUE_WHEEL)
            {
                page_number = ANT_BPWR_PAGE_17;
            }
            else // assumed TORQUE_CRANK
            {
                page_number = ANT_BPWR_PAGE_18;
            }
        }
        p_bpwr_cb->message_counter++;
    }

    return page_number;
}


/**@brief Function for encoding Bicycle Power Sensor message.
 *
 * @note Assume to be call each time when Tx window will occur.
 */
static void sens_message_encode(ant_bpwr_profile_t * p_profile, uint8_t * p_message_payload)
{
    ant_bpwr_message_layout_t * p_bpwr_message_payload =
        (ant_bpwr_message_layout_t *)p_message_payload;

    p_bpwr_message_payload->page_number = next_page_number_get(p_profile);

    LOG_INF("B-PWR tx page:                              %u", p_bpwr_message_payload->page_number);

    switch (p_bpwr_message_payload->page_number)
    {
        case ANT_BPWR_PAGE_1:
            ant_bpwr_page_1_encode(p_bpwr_message_payload->page_payload, &(p_profile->page_1));
            break;

        case ANT_BPWR_PAGE_16:
            ant_bpwr_page_16_encode(p_bpwr_message_payload->page_payload, &(p_profile->page_16));
            ant_bpwr_cadence_encode(p_bpwr_message_payload->page_payload, &(p_profile->common));
            break;

        case ANT_BPWR_PAGE_17:
            ant_bpwr_page_17_encode(p_bpwr_message_payload->page_payload, &(p_profile->page_17));
            ant_bpwr_cadence_encode(p_bpwr_message_payload->page_payload, &(p_profile->common));
            break;

        case ANT_BPWR_PAGE_18:
            ant_bpwr_page_18_encode(p_bpwr_message_payload->page_payload, &(p_profile->page_18));
            ant_bpwr_cadence_encode(p_bpwr_message_payload->page_payload, &(p_profile->common));
            break;

        case ANT_COMMON_PAGE_80:
            ant_common_page_80_encode(p_bpwr_message_payload->page_payload, &(p_profile->page_80));
            break;

        case ANT_COMMON_PAGE_81:
            ant_common_page_81_encode(p_bpwr_message_payload->page_payload, &(p_profile->page_81));
            break;

        default:
            return;
    }

    p_profile->evt_handler(p_profile, (ant_bpwr_evt_t)p_bpwr_message_payload->page_number);

}


/**@brief Function for decoding messages received by Bicycle Power sensor message.
 *
 * @note Assume to be call each time when Rx window will occur.
 */
static void sens_message_decode(ant_bpwr_profile_t * p_profile, uint8_t * p_message_payload)
{
    const ant_bpwr_message_layout_t * p_bpwr_message_payload =
        (ant_bpwr_message_layout_t *)p_message_payload;
    ant_bpwr_page1_data_t page1;

    switch (p_bpwr_message_payload->page_number)
    {
        case ANT_BPWR_PAGE_1:
            ant_bpwr_page_1_decode(p_bpwr_message_payload->page_payload, &page1);
            p_profile->_cb.p_sens_cb->calib_stat = BPWR_SENS_CALIB_REQUESTED;
            p_profile->_cb.p_sens_cb->calib_handler(p_profile, &page1);
            break;

        default:
            break;
    }
}


/**@brief Function for decoding messages received by Bicycle Power display message.
 *
 * @note Assume to be call each time when Rx window will occur.
 */
static void disp_message_decode(ant_bpwr_profile_t * p_profile, uint8_t * p_message_payload)
{
    const ant_bpwr_message_layout_t * p_bpwr_message_payload =
        (ant_bpwr_message_layout_t *)p_message_payload;

    LOG_INF("B-PWR rx page:                              %u", p_bpwr_message_payload->page_number);

    switch (p_bpwr_message_payload->page_number)
    {
        case ANT_BPWR_PAGE_1:
            ant_bpwr_page_1_decode(p_bpwr_message_payload->page_payload, &(p_profile->page_1));
            p_profile->_cb.p_disp_cb->calib_stat = BPWR_DISP_CALIB_NONE;
            break;

        case ANT_BPWR_PAGE_16:
            ant_bpwr_page_16_decode(p_bpwr_message_payload->page_payload, &(p_profile->page_16));
            ant_bpwr_cadence_decode(p_bpwr_message_payload->page_payload, &(p_profile->common));
            break;

        case ANT_BPWR_PAGE_17:
            ant_bpwr_page_17_decode(p_bpwr_message_payload->page_payload, &(p_profile->page_17));
            ant_bpwr_cadence_decode(p_bpwr_message_payload->page_payload, &(p_profile->common));
            break;

        case ANT_BPWR_PAGE_18:
            ant_bpwr_page_18_decode(p_bpwr_message_payload->page_payload, &(p_profile->page_18));
            ant_bpwr_cadence_decode(p_bpwr_message_payload->page_payload, &(p_profile->common));
            break;

        case ANT_COMMON_PAGE_80:
            ant_common_page_80_decode(p_bpwr_message_payload->page_payload, &(p_profile->page_80));
            break;

        case ANT_COMMON_PAGE_81:
            ant_common_page_81_decode(p_bpwr_message_payload->page_payload, &(p_profile->page_81));
            break;

        default:
            return;
    }

    p_profile->evt_handler(p_profile, (ant_bpwr_evt_t)p_bpwr_message_payload->page_number);
}

int  ant_bpwr_calib_request(ant_bpwr_profile_t * p_profile, ant_bpwr_page1_data_t * p_page_1)
{
    ant_bpwr_message_layout_t bpwr_message_payload;

    if (p_profile->_cb.p_disp_cb->calib_stat == BPWR_DISP_CALIB_REQUESTED)
    {
        return NRF_ANT_SUCCESS; // calibration in progress, so omit this request
    }

    bpwr_message_payload.page_number = ANT_BPWR_PAGE_1;
    ant_bpwr_page_1_encode(bpwr_message_payload.page_payload, p_page_1);

    int err = ant_acknowledge_message_tx(p_profile->channel_number,
                                                      sizeof (bpwr_message_payload),
                                                      (uint8_t *) &bpwr_message_payload);

    if (err == NRF_ANT_SUCCESS) {
        p_profile->_cb.p_disp_cb->calib_timeout = BPWR_CALIB_INT_TIMEOUT; // initialize watch on calibration's time-out
        p_profile->_cb.p_disp_cb->calib_stat    = BPWR_DISP_CALIB_REQUESTED;
        LOG_INF("Start calibration process");
    }

    return err;
}


void ant_bpwr_calib_response(ant_bpwr_profile_t * p_profile)
{
    if (p_profile->_cb.p_sens_cb->calib_stat != BPWR_SENS_CALIB_READY) // abort if callback request is in progress
    {
        p_profile->_cb.p_sens_cb->calib_stat = BPWR_SENS_CALIB_READY; // calibration respond
    }
}


/**@brief Function for hangling calibration events.
 */
static void service_calib(ant_bpwr_profile_t * p_profile, uint8_t event)
{
    ant_bpwr_evt_t       bpwr_event;

    if (p_profile->_cb.p_disp_cb->calib_stat == BPWR_DISP_CALIB_REQUESTED)
    {
        switch (event)
        {
            case EVENT_RX:
            /* fall through */
            case EVENT_RX_FAIL:

                if (p_profile->_cb.p_disp_cb->calib_timeout-- == 0)
                {
                    bpwr_event = ANT_BPWR_CALIB_TIMEOUT;
                    break;
                }
                else
                {
                    return;
                }

            case EVENT_TRANSFER_TX_FAILED:
                bpwr_event = ANT_BPWR_CALIB_REQUEST_TX_FAILED;
                break;

            case EVENT_RX_SEARCH_TIMEOUT:
                bpwr_event = ANT_BPWR_CALIB_TIMEOUT;
                break;

            default:
                return;
        }

        LOG_INF("End calibration process");
        p_profile->_cb.p_disp_cb->calib_stat = BPWR_DISP_CALIB_NONE;

        p_profile->evt_handler(p_profile, bpwr_event);
    }
}


static void ant_message_send(ant_bpwr_profile_t * p_profile)
{
    uint8_t  p_message_payload[ANT_STANDARD_DATA_PAYLOAD_SIZE];

    sens_message_encode(p_profile, p_message_payload);

    int err_code = ant_broadcast_message_tx(p_profile->channel_number,
                                sizeof (p_message_payload),
                                p_message_payload);

    __ASSERT_NO_MSG(err_code == 0);
}


int ant_bpwr_disp_open(ant_bpwr_profile_t * p_profile)
{
    __ASSERT_NO_MSG(p_profile != NULL);

    LOG_INF("ANT B-PWR %u open", p_profile->channel_number);
    return ant_channel_open(p_profile->channel_number);
}


int ant_bpwr_sens_open(ant_bpwr_profile_t * p_profile)
{
    __ASSERT_NO_MSG(p_profile != NULL);

    // Fill tx buffer for the first frame
    ant_message_send(p_profile);

    LOG_INF("ANT B-PWR %u open", p_profile->channel_number);
    return ant_channel_open(p_profile->channel_number);
}


void ant_bpwr_sens_evt_handler(ant_evt_t * p_ant_event, void * p_context)
{
    ant_bpwr_profile_t * p_profile = ( ant_bpwr_profile_t *)p_context;

    if (p_ant_event->channel == p_profile->channel_number)
    {
        switch (p_ant_event->event)
        {
            case EVENT_TX:
                ant_message_send(p_profile);
                break;

            case EVENT_RX:
                if (p_ant_event->message.ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID)
                {
                    sens_message_decode(p_profile, p_ant_event->message.ANT_MESSAGE_aucPayload);
                }
                break;

            default:
                // No implementation needed
                break;
        }
    }
}


void ant_bpwr_disp_evt_handler(ant_evt_t * p_ant_event, void * p_context)
{
    ant_bpwr_profile_t * p_profile = ( ant_bpwr_profile_t *)p_context;

    if (p_ant_event->channel == p_profile->channel_number)
    {
        switch (p_ant_event->event)
        {
            case EVENT_RX:

                if (p_ant_event->message.ANT_MESSAGE_ucMesgID == MESG_BROADCAST_DATA_ID
                 || p_ant_event->message.ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID
                 || p_ant_event->message.ANT_MESSAGE_ucMesgID == MESG_BURST_DATA_ID)
                {
                    disp_message_decode(p_profile, p_ant_event->message.ANT_MESSAGE_aucPayload);
                }
                break;

            default:
                break;
        }
        service_calib(p_profile, p_ant_event->event);
    }
}

