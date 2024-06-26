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
 * This file incorporates work covered by the following copyrights and permission notices:
 * zephyr/samples/bluetooth/peripheral_hr/src/main.c
 * Copyright (c) 2015-2016 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/hrs.h>

#include <ant_parameters.h>
#include <ant_state_indicator.h>
#include <ant_key_manager.h>
#include <ant_profiles/hrm/ant_hrm.h>
#include "ant_interface.h"

#include <dk_buttons_and_leds.h>

#define STACKSIZE 1024
#define PRIORITY 7
#define HRS_QUEUE_SIZE 16

LOG_MODULE_REGISTER(hrm_relay, LOG_LEVEL_INF);

static struct bt_conn *default_conn;

HRM_DISP_CHANNEL_CONFIG_DEF(hrm, CONFIG_HRM_RX_CHANNEL_NUM, CONFIG_HRM_RX_CHAN_ID_TRANS_TYPE,
                            CONFIG_HRM_RX_CHAN_ID_DEV_NUM, CONFIG_HRM_RX_NETWORK_NUM,
                            HRM_MSG_PERIOD_4Hz);

static ant_hrm_profile_t hrm;

static void ant_hrm_evt_handler(ant_hrm_profile_t *p_profile, ant_hrm_evt_t event);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_HRS_VAL),
                  BT_UUID_16_ENCODE(BT_UUID_DIS_VAL)),
    BT_DATA_BYTES(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME)
};

K_MSGQ_DEFINE(hrs_queue, sizeof(ant_hrm_profile_t), HRS_QUEUE_SIZE, 4);

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static void connected(struct bt_conn *conn, uint8_t err) {
  LOG_INF("Connected");

  if (default_conn == NULL) {
    default_conn = bt_conn_ref(conn);
  }

  if (conn != default_conn) {
    LOG_ERR("Connection failed (err 0x%02x)", err);
    return;
  }
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
  int err;

  LOG_INF("Disconnected (reason 0x%02x)", reason);
  if (conn != default_conn) {
    return;
  }

  bt_conn_unref(default_conn);
  default_conn = NULL;

  // Advertising will be restarted and the ANT channel will be reopened when
  // ANT event CHANNEL_CLOSED is received.
  err = ant_channel_close(CONFIG_HRM_RX_CHANNEL_NUM);
  if (err) {
    LOG_INF("ant_channel_close failed (err %x)", err);
    return;
  }
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

static int bt_ready(void) {
  int err;

  LOG_INF("Bluetooth initialized");

  err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
  if (err && err != -EALREADY) {
    LOG_INF("Advertising failed to start (err %x)", err);
    return err;
  }

  LOG_INF("Advertising started");
  return 0;
}

static void auth_cancel(struct bt_conn *conn) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Pairing cancelled: %s", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
    .cancel = auth_cancel,
};

static void hrs_notify_thread(void) {
  ant_hrm_profile_t hrm_profile;

  while (1) {
    k_msgq_get(&hrs_queue, &hrm_profile, K_FOREVER);
    bt_hrs_notify((uint16_t)hrm_profile.page_0.computed_heart_rate);
  }
}

/**@brief Start receiving the ANT HRM data.
 */
static int ant_hrm_rx_start(void) {
  int err = ant_hrm_disp_open(&hrm);
  if (err) {
    LOG_ERR("ant_hrm_disp_open failed: %d", err);
    return err;
  }
  ant_state_indicator_channel_opened();
  return err;
}

/**@brief Attempt to both open the ant channel and start BLE advertising.
 */
static int ant_and_adv_start(void) {
  int err = bt_ready();
  if (err) {
    LOG_ERR("bt_ready failed: %d", err);
    return err;
  }

  err = ant_hrm_rx_start();
  if (err) {
    LOG_ERR("ant_hrm_rx_start failed: %d", err);
    return err;
  }

  return err;
}

/**@brief Handle received ANT+ HRM data.
 *
 * @param[in]   p_profile       Pointer to the ANT+ HRM profile instance.
 * @param[in]   event           Event related with ANT+ HRM Display profile.
 */
static void ant_hrm_evt_handler(ant_hrm_profile_t *p_profile, ant_hrm_evt_t event) {
  int err;
  ant_hrm_profile_t hrm_profile = *p_profile;

  err = k_msgq_put(&hrs_queue, &hrm_profile, K_NO_WAIT);
  if (err) {
    LOG_ERR("ant_hrm_evt_handler: hrs_queue full, discarding data (err %d)\n", err);
  }
}

static int utils_setup(void) {
  int err = ant_state_indicator_init(hrm.channel_number, HRM_DISP_CHANNEL_TYPE);
  if (err) {
    LOG_ERR("ant_state_indicator_init failed: %d", err);
    return err;
  }

  return err;
}

/**@brief Function for handling a ANT stack event.
 *
 * @param[in] p_ant_evt  ANT stack event.
 */
static void ant_evt_handler(ant_evt_t *p_ant_evt) {
  ant_hrm_disp_evt_handler(p_ant_evt, &hrm);

  if (p_ant_evt->event != EVENT_CHANNEL_CLOSED) {
    return;
  }
  if (default_conn != NULL) {
    ant_hrm_rx_start();
  } else {
    ant_and_adv_start();
  }
}

static int profile_setup(void) {
  // Initialize ANT+ HRM receive channel.
  int err = ant_hrm_disp_init(&hrm, HRM_DISP_CHANNEL_CONFIG(hrm), ant_hrm_evt_handler);
  if (err) {
    LOG_ERR("ant_hrm_disp_init failed: %d", err);
    return err;
  }

  return err;
}

/**@brief Function for initializing the ANT stack.
 */
int ant_stack_setup(void) {
  int err = ant_init();
  if (err) {
    LOG_ERR("ant_init failed: %d", err);
    return err;
  }
  LOG_INF("ANT Version %s", ANT_VERSION_STRING);

  err = ant_cb_register(&ant_evt_handler);
  if (err) {
    LOG_ERR("ant_cb_register failed: %d", err);
    return err;
  }

  err = ant_plus_key_set(CONFIG_HRM_RX_NETWORK_NUM);
  if (err) {
    LOG_ERR("ant_plus_key_set failed: %d", err);
  }
  return err;
}

int main(void) {
  int err;

  LOG_INF("BLE ANT+ HRM Relay sample starting...");

  err = ant_stack_setup();
  if (err) {
    goto ERROR_EXIT;
  }

  err = utils_setup();
  if (err) {
    goto ERROR_EXIT;
  }

  err = profile_setup();
  if (err) {
    goto ERROR_EXIT;
  }

  err = bt_enable(NULL);
  if (err) {
    LOG_INF("Bluetooth init failed (err %d)", err);
    goto ERROR_EXIT;
  }

  err = ant_and_adv_start();
  if (err) {
    LOG_INF("ANT and BLE start failed (err %d)", err);
    goto ERROR_EXIT;
  }

  bt_conn_auth_cb_register(&auth_cb_display);

  return 0;

ERROR_EXIT:
  ant_state_indicator_fatal_error();
  k_oops();

  return 0;
}

K_THREAD_DEFINE(hrs_notify_thread_id, STACKSIZE, hrs_notify_thread, NULL, NULL, NULL, PRIORITY, 0,
                0);
