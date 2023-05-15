/**
 * This software is subject to the ANT+ Shared Source License
 * www.thisisant.com/swlicenses
 * Copyright (c) Garmin Canada Inc. 2022
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 *    1) Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer.
 *
 *    2) Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 *    3) Neither the name of Garmin nor the names of its
 *       contributors may be used to endorse or promote products
 *       derived from this software without specific prior
 *       written permission.
 *
 * The following actions are prohibited:
 *
 *    1) Redistribution of source code containing the ANT+ Network
 *       Key. The ANT+ Network Key is available to ANT+ Adopters.
 *       Please refer to http://thisisant.com to become an ANT+
 *       Adopter and access the key.
 *
 *    2) Reverse engineering, decompilation, and/or disassembly of
 *       software provided in binary form under this license.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE HEREBY
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; DAMAGE TO ANY DEVICE, LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE. SOME STATES DO NOT ALLOW
 * THE EXCLUSION OF INCIDENTAL OR CONSEQUENTIAL DAMAGES, SO THE
 * ABOVE LIMITATIONS MAY NOT APPLY TO YOU.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ant_host_init.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_error.h"

#include "ant_channel_config.h"
#include "ant_search_config.h"
#include "ant_state_indicator.h"

LOG_MODULE_REGISTER(ant_background_scanning, LOG_LEVEL_INF);

#define ANT_BEACON_PAGE             ((uint8_t) 1)

void ant_message_send(void);
static void background_scanner_process(ant_evt_t * p_ant_evt);
static void master_beacon_process(ant_evt_t * p_ant_evt);

static uint8_t m_last_rssi       = 0;
static uint16_t m_last_device_id = 0;
static uint8_t m_received = 0;

/**@brief Function for setting payload for ANT message and sending it via
 *        ANT master beacon channel.
 *
 * @details This function is called from the ANT stack event interrupt handler after an ANT stack
 *          event has been received.
 *
 * @details   ANT_BEACON_PAGE message is queued. The format is:
 *            byte[0]   = page (1 = ANT_BEACON_PAGE)
 *            byte[1]   = last RSSI value received
 *            byte[2-3] = channel ID of device corresponding to last RSSI value (little endian)
 *            byte[6]   = counter that increases with every message period
 *            byte[7]   = number of messages received on background scanning channel
 */
void ant_message_send()
{
    ant_err_t       err_code;
    uint8_t        tx_buffer[ANT_STANDARD_DATA_PAYLOAD_SIZE];
    static uint8_t counter = 0;

    tx_buffer[0] = ANT_BEACON_PAGE;
    tx_buffer[1] = m_last_rssi;
    tx_buffer[2] = (uint8_t) m_last_device_id;        // LSB
    tx_buffer[3] = (uint8_t) (m_last_device_id >> 8); // MSB
    tx_buffer[6] = counter++;
    tx_buffer[7] = m_received;

    err_code = ant_broadcast_message_tx(CONFIG_ANT_MS_CHANNEL_NUMBER,
                                           ANT_STANDARD_DATA_PAYLOAD_SIZE,
                                           tx_buffer);
    if (err_code) {
        LOG_ERR("ant_broadcast_message_tx failed!");
    }
}


/**@brief Initialize application.
 */
static ant_err_t application_initialize()
{
    /* Set library config to report RSSI and Device ID */
    ant_err_t err_code = ant_lib_config_set(ANT_LIB_CONFIG_MESG_OUT_INC_RSSI
                                            | ANT_LIB_CONFIG_MESG_OUT_INC_DEVICE_ID);
    if (err_code) {
        LOG_ERR("ant_lib_config_set() failed: 0x%X", err_code);
        return err_code;
    }

    const ant_channel_config_t ms_channel_config =
    {
        .channel_number    = CONFIG_ANT_MS_CHANNEL_NUMBER,
        .channel_type      = CHANNEL_TYPE_MASTER,
        .ext_assign        = 0x00,
        .rf_freq           = CONFIG_RF_FREQ,
        .transmission_type = CONFIG_CHAN_ID_TRANS_TYPE,
        .device_type       = CONFIG_CHAN_ID_DEV_TYPE,
        .device_number     = CONFIG_CHAN_ID_DEV_NUM,
        .channel_period    = CONFIG_CHAN_PERIOD,
        .network_number    = CONFIG_ANT_NETWORK_NUM,
    };

    const ant_channel_config_t bs_channel_config =
    {
        .channel_number    = CONFIG_ANT_BS_CHANNEL_NUMBER,
        .channel_type      = CHANNEL_TYPE_SLAVE,
        .ext_assign        = EXT_PARAM_ALWAYS_SEARCH,
        .rf_freq           = CONFIG_RF_FREQ,
        .transmission_type = CONFIG_CHAN_ID_TRANS_TYPE,
        .device_type       = CONFIG_CHAN_ID_DEV_TYPE,
        .device_number     = 0x00,              // Wild card
        .channel_period    = 0x00,              // This is not taken into account.
        .network_number    = CONFIG_ANT_NETWORK_NUM,
    };

    const ant_search_config_t bs_search_config =
    {
        .channel_number        = CONFIG_ANT_BS_CHANNEL_NUMBER,
        .low_priority_timeout  = ANT_LOW_PRIORITY_TIMEOUT_DISABLE,
        .high_priority_timeout = ANT_HIGH_PRIORITY_SEARCH_DISABLE,
        .search_sharing_cycles = ANT_SEARCH_SHARING_CYCLES_DISABLE,
        .search_priority       = ANT_SEARCH_PRIORITY_DEFAULT,
        .waveform              = ANT_WAVEFORM_DEFAULT,
    };

    err_code = ant_channel_init(&ms_channel_config);
    if (err_code) {
        LOG_ERR("ant_channel_init() failed: 0x%X", err_code);
        return err_code;
    }

    err_code = ant_channel_init(&bs_channel_config);
    if (err_code) {
        LOG_ERR("ant_channel_init() failed: 0x%X", err_code);
        return err_code;
    }

    err_code = ant_search_init(&bs_search_config);
    if (err_code) {
        LOG_ERR("ant_search_init() failed: 0x%X", err_code);
        return err_code;
    }

    // Fill tx buffer for the first frame
    ant_message_send();

    err_code = ant_channel_open(CONFIG_ANT_MS_CHANNEL_NUMBER);
    if (err_code) {
        LOG_ERR("ant_channel_open() failed: 0x%X", err_code);
        return err_code;
    }

    err_code = ant_channel_open(CONFIG_ANT_BS_CHANNEL_NUMBER);
    if (err_code) {
        LOG_ERR("ant_channel_open() failed: 0x%X", err_code);
        return err_code;
    }
    ant_state_indicator_channel_opened();

    return 0;
}


/**@brief Process ANT message on ANT background scanning channel.
 *
 * @param[in] p_ant_evt   ANT message content.
 */
static void background_scanner_process(ant_evt_t * p_ant_evt)
{
    if (p_ant_evt->channel != CONFIG_ANT_BS_CHANNEL_NUMBER)
    {
        return;
    }

    switch (p_ant_evt->event)
    {
        case EVENT_RX:
        {
            if (p_ant_evt->message.ANT_MESSAGE_stExtMesgBF.bANTDeviceID)
            {
                // assumes little endian
                m_last_device_id = (p_ant_evt->message.ANT_MESSAGE_aucExtData[1]) << 8 | (p_ant_evt->message.ANT_MESSAGE_aucExtData[0]);
            }

            if (p_ant_evt->message.ANT_MESSAGE_stExtMesgBF.bANTRssi)
            {
                m_last_rssi = p_ant_evt->message.ANT_MESSAGE_aucExtData[5];
            }

            LOG_INF("=============================================================");
            LOG_INF("Message number %d", m_received);
            LOG_INF("Device ID:     %d", m_last_device_id);
            LOG_INF("RSSI:          %d", m_last_rssi);

            m_received++;
            break;
        }
        default:
        {
            break;
        }
    }
}


/**@brief Process ANT message on ANT master beacon channel.
 *
 *
 * @details   This function handles all events on the master beacon channel.
 *            On EVENT_TX an ANT_BEACON_PAGE message is queued.
 *
 * @param[in] p_ant_evt   ANT message content.
 */
static void master_beacon_process(ant_evt_t * p_ant_evt)
{
    if (p_ant_evt->channel != CONFIG_ANT_MS_CHANNEL_NUMBER)
    {
        return;
    }

    switch (p_ant_evt->event)
    {
        case EVENT_TX:
        {
            ant_message_send();
            break;
        }
        default:
        {
            break;
        }
    }
}


/**@brief Function for handling a ANT stack event.
 *
 * @param[in] p_ant_evt  ANT stack event.
 */
static void ant_evt_handler(ant_evt_t* p_ant_evt) {
    background_scanner_process(p_ant_evt);
    master_beacon_process(p_ant_evt);
}


/**@brief Function for ANT stack initialization.
 */
ant_err_t ant_stack_setup(void) {
    ant_err_t err_code;

    err_code = ant_init();
    if (err_code == 0) {
        LOG_INF("ANT Version %s", ANT_VERSION_STRING);
    } else {
        LOG_ERR("ant_init failed: 0x%X", err_code);
    }

    err_code = ant_cb_register(&ant_evt_handler);
    if (err_code) {
        LOG_ERR("ant_cb_register() failed: 0x%X", err_code);
    }

    return err_code;
}


static int utils_setup(void)
{
    int err = ant_state_indicator_init(CONFIG_ANT_BS_CHANNEL_NUMBER, CHANNEL_TYPE_SLAVE);
    if (err) {
        LOG_ERR("ant_state_indicator_init failed: %d", err);
        return err;
    }

    return err;
}


/* Main function */
int main(void)
{
    LOG_INF("Application starting...");

    ant_err_t err_code;

    err_code = ant_stack_setup();
    if (err_code) {
        LOG_ERR("ant_stack_setup() failed: 0x%X", err_code);
        goto ERROR_EXIT;
    }

    err_code = utils_setup();
    if (err_code) {
        goto ERROR_EXIT;
    }

    err_code = application_initialize();
    if (err_code) {
        goto ERROR_EXIT;
    }

    LOG_INF("ANT Background Scanning example started.");

    // Enter main loop
    for (;;) {
        k_sleep(K_MSEC(1000));
    }

ERROR_EXIT:
    ant_state_indicator_fatal_error();
    k_oops();

    return 0;
}
