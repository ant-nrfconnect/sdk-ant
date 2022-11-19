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

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_init.h"
#include "ant_channel_config.h"

#include "ant_advanced_burst.h"

// sets configured channel as MASTER or SLAVE.
#define CONFIGURE_AS_SLAVE 0

// burst test trigger defines.
#define BURST_START_AFTER_X_EVTS 25u    ///< start burst transfer after x TX_EVENTS if MASTER or x RX_EVENTS if SLAVE
static uint8_t m_start_burst_cnt = 0;

// Miscellaneous defines.
#define BURST_PACKET_SIZE       8u                              ///< The burst packet size, in bytes.
#define BURST_PACKET_TOTAL_SIZE (BURST_PACKET_SIZE + 1u)        ///< The burst packet total size in bytes, including channel number/sequence number.

#define BURST_TOTAL_PACKETS     1024u                           ///< Total size of burst data to send.
#define BURST_BLOCK_SIZE        CONFIG_ANT_BURST_QUEUE_SIZE     ///< Burst block size, in number of bytes.

static uint8_t  m_sequence_number;                ///< Burst sequence number.
static uint8_t  m_burst_data[BURST_BLOCK_SIZE];   ///< Burst buffer.
static uint32_t m_bytes;                          ///< Counter for number of transmitted bytes.
static uint8_t  m_counter;                        ///< Counter for populating dummy data.
static bool     m_burst_rx;                       ///< Flag to indicate if we are receiving a burst.

static void ant_evt_handler_burst(ant_evt_t * p_ant_evt);

LOG_MODULE_REGISTER(ant_advanced_burst, LOG_LEVEL_INF);

/**@brief Fill burst buffer with dummy data
 *
 * @param[in] p_ buffer Pointer to the burst buffer.
 * @param[in] size      Size of the burst buffer.
 */
static void burst_buffer_fill(uint8_t * p_buffer, uint16_t size) {
  // Fill up burst buffer with dummy data
  for (uint8_t i = 0; i < size; i++) {
    p_buffer[i] = m_counter;
    m_counter++;
  }
}

/**@brief Function to send burst data
 */
static void burst_data_send(void) {
  uint32_t err_code;
  uint8_t  burst_segment = BURST_SEGMENT_CONTINUE;
  uint8_t  bytes_to_send = BURST_BLOCK_SIZE;

  if (m_bytes >= BURST_TOTAL_PACKETS * BURST_PACKET_SIZE) {
    // There is no more data to send
    return;
  }

  if (m_bytes == 0) {
    // This is the first block
    burst_segment = BURST_SEGMENT_START;
  }

  if ((BURST_TOTAL_PACKETS * BURST_PACKET_SIZE - m_bytes) <= BURST_BLOCK_SIZE) {
    // This is the last block
    burst_segment |= BURST_SEGMENT_END;
    bytes_to_send  = BURST_TOTAL_PACKETS * BURST_PACKET_SIZE - m_bytes;
  }

  memset(m_burst_data, 0, BURST_BLOCK_SIZE);
  burst_buffer_fill(m_burst_data, bytes_to_send);

  err_code = ant_burst_handler_request(CONFIG_ADVANCED_BURST_TX_CHANNEL_NUM,
                                       bytes_to_send,
                                       m_burst_data,
                                       burst_segment);
  if (err_code) {
    LOG_ERR("ant_burst_handler_request() failed: 0x%X", err_code);
  }
  m_bytes += bytes_to_send;
}

/**@brief Function to handle received burst data
 *
 * @param[in] p_burst_message  Buffer containing burst message, including channel/sequence number.
 */
static void burst_data_process(uint8_t * p_burst_message) {
  // If this is the first packet
  if ((p_burst_message[0] & SEQUENCE_NUMBER_MASK) == SEQUENCE_FIRST_MESSAGE) {
    m_burst_rx = true;
  // If this is the last packet
  } else if ((p_burst_message[0] & SEQUENCE_LAST_MESSAGE) != 0) {
    m_burst_rx = false;
  }
  // The burst data is available in p_burst_message[1] to p_burst_message[8]
}

ant_err_t ant_advanced_burst_setup(void)
{
  ant_err_t err_code;

  m_counter  = 0;
  m_bytes    = 0;
  m_burst_rx = false;

  ant_channel_config_t channel_config = {
      .channel_number     = CONFIG_ADVANCED_BURST_TX_CHANNEL_NUM,
      .channel_type       = CHANNEL_TYPE_MASTER,
      .ext_assign         = 0x00,
      .rf_freq            = CONFIG_ADVANCED_BURST_TX_RF_FREQ,
      .transmission_type  = CONFIG_ADVANCED_BURST_TX_CHAN_ID_TRANS_TYPE,
      .device_type        = CONFIG_ADVANCED_BURST_TX_CHAN_ID_DEV_TYPE,
      .device_number      = CONFIG_ADVANCED_BURST_TX_CHAN_ID_DEV_NUM,
      .channel_period     = CONFIG_ADVANCED_BURST_TX_CHAN_PERIOD,
      .network_number     = CONFIG_ADVANCED_BURST_TX_NETWORK_NUM,
  };

  uint8_t adv_burst_config[] = {
      ADV_BURST_MODE_ENABLE,
      ADV_BURST_MODES_MAX_SIZE,
      0,
      0,
      0,
      0,
      0,
      0
  };

#if CONFIGURE_AS_SLAVE
    channel_config.channel_type  = CHANNEL_TYPE_SLAVE;
    channel_config.device_number = 0; // Wild card
#endif

  err_code = ant_cb_register(&ant_evt_handler_burst);
  if (err_code) {
    LOG_ERR("ant_cb_register() failed: 0x%X", err_code);
    return err_code;
  }

  err_code = ant_channel_init(&channel_config);
  if (err_code) {
    LOG_ERR("ant_channel_init() failed: 0x%X", err_code);
    return err_code;
  }

  err_code = ant_adv_burst_config_set(adv_burst_config, sizeof(adv_burst_config));
  if (err_code) {
    LOG_ERR("ant_adv_burst_config_set() failed: 0x%X", err_code);
    return err_code;
  }

  err_code = ant_channel_open(CONFIG_ADVANCED_BURST_TX_CHANNEL_NUM);
  if (err_code) {
    LOG_ERR("ant_channel_open() failed: 0x%X", err_code);
    return err_code;
  }

  return 0;
}

/**
 * @brief Function for handling ANT events for burst transfers.
 *        Events of interest by the burst transfer are processed here.
 *
 * @param[in]   p_ant_evt       Event received from the ANT stack.
 */
static void ant_evt_handler_burst(ant_evt_t * p_ant_evt)
{
  uint8_t p_burst_message[BURST_PACKET_TOTAL_SIZE];

  switch (p_ant_evt->event) {
    case EVENT_TRANSFER_TX_START:
        break;

    case EVENT_TRANSFER_NEXT_DATA_BLOCK:
        // Send the next block of data
        burst_data_send();
        break;

    case EVENT_TRANSFER_TX_COMPLETED:
        m_bytes = 0;
        m_start_burst_cnt = 0;
        break;

    case EVENT_TRANSFER_TX_FAILED:
        m_bytes = 0;
        m_start_burst_cnt = 0;
        break;

    case EVENT_TRANSFER_RX_FAILED:
        m_burst_rx = false;
        break;

    case EVENT_TX:
        // After x EVENT_TXs as MASTER, start burst transfer
        if (m_start_burst_cnt++ == BURST_START_AFTER_X_EVTS) {
          // Do not start a new burst if there is one in progress
          if (m_bytes == 0 && !m_burst_rx) {
            m_counter = 0;
            burst_data_send();
          }
        }
        break;

    case EVENT_RX:
      switch (p_ant_evt->message.ANT_MESSAGE_ucMesgID) {
        // After x EVENT_TXs as SLAVE, start burst transfer
        case MESG_BROADCAST_DATA_ID:
          if (m_start_burst_cnt++ == BURST_START_AFTER_X_EVTS) {
            // Do not start a new burst if there is one in progress
            if (m_bytes == 0 && !m_burst_rx) {
              m_counter = 0;
              burst_data_send();
            }
          }
          break;

        case MESG_BURST_DATA_ID:
          // The first packet of an advanced burst transfer will be a regular burst
          // message. If the other end point does not have advanced burst enabled,
          // we will receive regular burst messages as well.
          p_burst_message[0] = p_ant_evt->message.ANT_MESSAGE_ucChannel;
          memcpy(&p_burst_message[1],
                 p_ant_evt->message.ANT_MESSAGE_aucPayload,
                 BURST_PACKET_SIZE);
          m_sequence_number = SEQUENCE_NUMBER_MASK & p_ant_evt->message.ANT_MESSAGE_ucChannel;
          burst_data_process(p_burst_message);
          break;

        case MESG_ADV_BURST_DATA_ID:
          // If it is an advanced burst message, split it into a series of regular 8-byte
          // burst messages so everything can be processed in the same function
          for (uint8_t i = 0; (int32_t)(i * BURST_PACKET_SIZE) < (p_ant_evt->message.ANT_MESSAGE_ucSize - 1); i++ ) {
              // For each 8-byte packet
              // Increment the sequence number
              if (m_sequence_number == SEQUENCE_NUMBER_ROLLOVER) {
                  m_sequence_number = 0;
              }
              m_sequence_number += SEQUENCE_NUMBER_INC;

              // Check if it is the last packet
              if (((p_ant_evt->message.ANT_MESSAGE_ucChannel & SEQUENCE_LAST_MESSAGE) != 0) &&
                  (((i + 1) * BURST_PACKET_SIZE) == (p_ant_evt->message.ANT_MESSAGE_ucSize - 1))) {
                  m_sequence_number |= SEQUENCE_LAST_MESSAGE;
              }

              // Package up the burst message data as a regular burst, replacing
              // the sequence number.
              p_burst_message[0] = m_sequence_number | (p_ant_evt->message.ANT_MESSAGE_ucChannel & CHANNEL_NUMBER_MASK);
              memcpy(&p_burst_message[1],
                     &p_ant_evt->message.ANT_MESSAGE_aucMesgData[i * BURST_PACKET_SIZE + 1],
                     BURST_PACKET_SIZE);

              // Process burst data
              burst_data_process(p_burst_message);
          }
          break;
      }
      break;

    default:
        break; // no implementation needed
  }
}
