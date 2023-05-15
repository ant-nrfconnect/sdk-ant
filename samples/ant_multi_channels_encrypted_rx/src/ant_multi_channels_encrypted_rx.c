/**
 * This software is subject to the ANT+ Shared Source License
 * www.thisisant.com/swlicenses
 * Copyright (c) Garmin Canada Inc. 2023
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

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_multi_channels_encrypted_rx.h"
#include "ant_host_init.h"
#include "ant_channel_config.h"
#include "ant_search_config.h"
#include "ant_encrypt_negotiation_slave.h"

LOG_MODULE_REGISTER(ant_multi_channels_encrypted_rx, LOG_LEVEL_INF);

// Channel configuration.
#define ANT_EXT_ASSIGN              0x00                    /**< ANT Ext Assign. */
#define ANT_CHANNEL_DEFAULT_NETWORK 0x00                    /**< ANT channel network. */
#define ANT_MSG_IDX_ID              1u                      /**< ANT message ID index. */

/** Encryption key. */
#define CRYPTO_KEY                  {0x03, 0x01, 0x04, 0x01, 0x05, 0x09, 0x02, 0x06, 0x05, 0x03, \
                                     0x05, 0x08, 0x09, 0x07, 0x09, 0x03}

#define UNIQUE_DEVICE_ID            {0x89, 0xAB, 0xCD, 0xEF};

/** Number of channels to open <1,CONFIG_ANT_ENCRYPTION_NUM_CHANNELS>. */
#define NUMBER_OF_CHANNELS_TO_OPEN CONFIG_ANT_ENCRYPTION_NUM_CHANNELS

void ant_se_num_of_decrypted_channels_display(void)
{
  uint8_t  num_decrypted_channels = 0;

  for (uint32_t i = 0; i < NUMBER_OF_CHANNELS_TO_OPEN; i++) {
    if (ant_channel_encrypt_tracking_state_get(i) == ANT_ENC_CHANNEL_STAT_TRACKING_DECRYPTED) {
      num_decrypted_channels++;
    }
  }
  LOG_INF("num_decrypted_channels: %d", num_decrypted_channels);
}

void ant_se_encrypt_track_handler(uint8_t ant_channel, ant_encrypt_user_evt_t event)
{
  ant_se_num_of_decrypted_channels_display();
}

ant_err_t ant_se_channel_rx_broadcast_setup(void) {
  uint32_t err_code;

  uint8_t crypto_key[] = CRYPTO_KEY;
  uint8_t device_id[] = UNIQUE_DEVICE_ID;

  ANT_ENCRYPT_STACK_SETTINGS_BASE_DEF(se_conf, crypto_key, device_id);

  ant_encrypt_channel_settings_t channel_crypto_settings = {
    .mode            = ENCRYPTION_BASIC_REQUEST_MODE,
    .key_index       = 0,
    .decimation_rate = 1
  };

  ant_channel_config_t m_channel_config = {
    .channel_type       = CHANNEL_TYPE_SLAVE,
    .ext_assign         = ANT_EXT_ASSIGN,
    .rf_freq            = CONFIG_RF_FREQ,
    .transmission_type  = CONFIG_ID_TRANS_TYPE,
    .device_type        = CONFIG_ID_DEV_TYPE,
    .channel_period     = CONFIG_CHAN_PERIOD,
    .network_number     = ANT_CHANNEL_DEFAULT_NETWORK,
    .p_crypto_settings  = &channel_crypto_settings
    // .device_number and .channel_number structure members are set below separately.
  };

  ant_search_config_t m_search_config = {
    // .device_number structure member is set below separately.
    .low_priority_timeout  = ANT_LOW_PRIORITY_TIMEOUT_DISABLE,
    .high_priority_timeout = CONFIG_ANT_HIGH_PRIORITY_TIMEOUT,
    .search_sharing_cycles = ANT_SEARCH_SHARING_CYCLES_DISABLE,
    .search_priority       = ANT_SEARCH_PRIORITY_DEFAULT,
    .waveform              = ANT_WAVEFORM_DEFAULT,
  };

  err_code = ant_stack_encryption_config(&ANT_ENCRYPT_STACK_SETTINGS_BASE(se_conf));
  if (err_code) {
    LOG_ERR("ant_stack_encryption_config() failed: 0x%X", err_code);
    return err_code;
  }

  ant_enc_event_handler_register(ant_se_encrypt_track_handler);

  for (uint32_t i = 0; i < NUMBER_OF_CHANNELS_TO_OPEN; i++) {
    // fill variable channel ID.
    m_channel_config.channel_number = i;
    m_search_config.channel_number  = i;
    m_channel_config.device_number  = i + 1;

    err_code = ant_channel_init(&m_channel_config);
    if (err_code) {
      LOG_ERR("ant_channel_init() failed: 0x%X", err_code);
      return err_code;
    }

    // Set search time-out
    err_code = ant_search_init(&m_search_config);
    if (err_code) {
      LOG_ERR("ant_search_init() failed: 0x%X", err_code);
      return err_code;
    }

    // Open channel
    err_code = ant_channel_open(i);
    if (err_code) {
      LOG_ERR("ant_channel_open() failed: 0x%X", err_code);
      return err_code;
    }
  }

  return 0;
}

void ant_se_event_handler(ant_evt_t * p_ant_event) {
  ant_encrypt_event_handler(p_ant_event);
}
