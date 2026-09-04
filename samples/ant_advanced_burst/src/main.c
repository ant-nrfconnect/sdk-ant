/**
 * This software is subject to the ANT+ Shared Source License
 * https://developer.garmin.com/ant-program/licensing/shared-source-license/
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
 *       Key.
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
#include "ant_host_init.h"
#include "ant_channel_config.h"
#include "ant_advanced_burst.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/**@brief Helper function for converting ANT message buffer to string.
 */
void convert_buf_to_hex_str(uint8_t* buf, uint8_t buf_size, char *hex_str) {
  // note: hex_str size is buf_size*3 + 1 to accommodate hex bytes string
  //       representation w/ spacing + null
   char* p_str = hex_str;
   for (uint8_t i = 0; i < buf_size; i++) {
      uint8_t upper = ((buf[i] >> 4) & 0x0F);
      uint8_t lower = ((buf[i] >> 0) & 0x0F);
      if (upper < 0x0A) {
         *p_str++ = upper + 0x30;
      } else {
         *p_str++ = upper - 0x0A + 0x41;
      }
      if (lower < 0x0A) {
         *p_str++ = lower + 0x30;
      } else {
         *p_str++ = lower - 0x0A + 0x41;
      }
      // space
      *p_str++ = 0x20;
   }
    // null terminate
   *p_str = '\0';
}

/**@brief Function for handling ANT stack events. Currently used
 *        to log all received events by the application
 *
 * @param[in] p_ant_evt  ANT stack event.
 */
static void ant_evt_handler_main(ant_evt_t *p_ant_evt)
{
  // hex_str size: 2 digit hex representations + spaces + null termination
  char hex_str[MESG_BUFFER_SIZE *3 + 1];

  if (p_ant_evt->event) {
    convert_buf_to_hex_str(p_ant_evt->message.ANT_MESSAGE_aucMessage,
                           p_ant_evt->message.ANT_MESSAGE_ucSize + MESG_SIZE_SIZE + MESG_ID_SIZE,
                           hex_str);
    }

  // print out applicable decoded events
  switch (p_ant_evt->event) {
    case EVENT_RX_SEARCH_TIMEOUT:
      LOG_INF("EVENT_RX_SEARCH_TIMEOUT - %s", hex_str);
      break;
    case EVENT_RX_FAIL:
      LOG_INF("EVENT_RX_FAIL - %s", hex_str);
      break;
    case EVENT_TX:
      LOG_INF("EVENT_TX - %s", hex_str);
      break;
    case EVENT_TRANSFER_RX_FAILED:
      LOG_INF("EVENT_TRANSFER_RX_FAILED - %s", hex_str);
      break;
    case EVENT_TRANSFER_TX_COMPLETED:
      LOG_INF("EVENT_TRANSFER_TX_COMPLETED - %s", hex_str);
      break;
    case EVENT_TRANSFER_TX_FAILED:
      LOG_INF("EVENT_TRANSFER_TX_FAILED - %s", hex_str);
      break;
    case EVENT_CHANNEL_CLOSED:
      LOG_INF("EVENT_CHANNEL_CLOSED - %s", hex_str);
      break;
    case EVENT_RX_FAIL_GO_TO_SEARCH:
      LOG_INF("EVENT_RX_FAIL_GO_TO_SEARCH - %s", hex_str);
      break;
    case EVENT_CHANNEL_COLLISION:
      LOG_INF("EVENT_CHANNEL_COLLISION - %s", hex_str);
      break;
    case EVENT_TRANSFER_TX_START:
      LOG_INF("EVENT_TRANSFER_TX_START - %s", hex_str);
      break;
    case EVENT_TRANSFER_NEXT_DATA_BLOCK:
      LOG_INF("EVENT_TRANSFER_NEXT_DATA_BLOCK - %s", hex_str);
      break;
    case EVENT_RX:
      LOG_INF("EVENT_RX - %s", hex_str);
      break;

    default:
      LOG_INF("%s", hex_str);
      break;
  }
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
    return err_code;
  }

  err_code = ant_cb_register(&ant_evt_handler_main);
  if (err_code) {
    LOG_ERR("ant_cb_register() failed: 0x%X", err_code);
  }

  return err_code;
}

int main(void) {
  ant_err_t err_code;

  err_code = ant_stack_setup();
  if (err_code) {
    LOG_ERR("ant_stack_setup() failed: 0x%X", err_code);
    goto ERROR_EXIT;
  }

  err_code = ant_advanced_burst_setup();
  if (err_code) {
    goto ERROR_EXIT;
  }

  LOG_INF("ANT Advanced Burst example started");

  return 0;

ERROR_EXIT:
  k_oops();

  return 0;
}
