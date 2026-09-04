/**
 * This software is subject to the ANT+ Shared Source License
 * https://developer.garmin.com/ant-program/licensing/shared-source-license/
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

#include <ant_key_manager.h>
#include <ant_parameters.h>
#include <ant_profiles/bpwr/ant_bpwr.h>
#include <ant_state_indicator.h>

#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(bpwr_rx, LOG_LEVEL_INF);

#define CALIBRATION_REQUEST DK_BTN1_MSK

void ant_bpwr_evt_handler(ant_bpwr_profile_t *p_profile, ant_bpwr_evt_t event);

BPWR_DISP_CHANNEL_CONFIG_DEF(bpwr, CONFIG_BPWR_RX_CHANNEL_NUM, CONFIG_BPWR_RX_CHAN_ID_TRANS_TYPE,
                             CONFIG_BPWR_RX_CHAN_ID_DEV_NUM, CONFIG_BPWR_RX_NETWORK_NUM);
BPWR_DISP_PROFILE_CONFIG_DEF(bpwr, ant_bpwr_evt_handler);

static ant_bpwr_profile_t bpwr;

void ant_bpwr_evt_handler(ant_bpwr_profile_t *p_profile, ant_bpwr_evt_t event) {
  switch (event) {
    case ANT_BPWR_PAGE_1_UPDATED:
      // calibration data received from sensor
      LOG_INF("Received calibration data");
      break;

    case ANT_BPWR_PAGE_16_UPDATED:
      /* fall through */
    case ANT_BPWR_PAGE_17_UPDATED:
      /* fall through */
    case ANT_BPWR_PAGE_18_UPDATED:
      /* fall through */
    case ANT_BPWR_PAGE_80_UPDATED:
      /* fall through */
    case ANT_BPWR_PAGE_81_UPDATED:
      // data actualization
      break;

    case ANT_BPWR_CALIB_TIMEOUT:
      // calibration request time-out
      LOG_INF("ANT_BPWR_CALIB_TIMEOUT");
      break;

    case ANT_BPWR_CALIB_REQUEST_TX_FAILED:
      // Please consider retrying the request.
      LOG_INF("ANT_BPWR_CALIB_REQUEST_TX_FAILED");
      break;

    default:
      // never occurred
      break;
  }
}

static void button_changed(uint32_t button_state, uint32_t has_changed) {
  ant_bpwr_page1_data_t page1 = ANT_BPWR_GENERAL_CALIB_REQUEST();

  if (has_changed & CALIBRATION_REQUEST) {
    if (button_state & CALIBRATION_REQUEST) {
      // manual calibration request
      int err = ant_bpwr_calib_request(&bpwr, &page1);
      if (err) {
        LOG_ERR("Cannot start calibration process: %d", err);
      }
    }
  }
}

static int utils_setup(void) {
  int err = ant_state_indicator_init(bpwr.channel_number, BPWR_DISP_CHANNEL_TYPE);

  if (err) {
    LOG_ERR("ant_state_indicator_init failed: %d", err);
    return err;
  }

  err = dk_buttons_init(button_changed);
  if (err) {
    LOG_ERR("Cannot init buttons (err: %d)", err);
  }

  return err;
}

static void ant_evt_handler(ant_evt_t *p_ant_evt) { ant_bpwr_disp_evt_handler(p_ant_evt, &bpwr); }

static int profile_setup(void) {
  int err =
      ant_bpwr_disp_init(&bpwr, BPWR_DISP_CHANNEL_CONFIG(bpwr), BPWR_DISP_PROFILE_CONFIG(bpwr));
  if (err) {
    LOG_ERR("ant_bpwr_disp_init failed: %d", err);
    return err;
  }

  err = ant_bpwr_disp_open(&bpwr);
  if (err) {
    LOG_ERR("ant_bpwr_disp_open failed: %d", err);
    return err;
  }

  ant_state_indicator_channel_opened();
  return err;
}

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

  err = ant_plus_key_set(CONFIG_BPWR_RX_NETWORK_NUM);
  if (err) {
    LOG_ERR("ant_plus_key_set failed: %d", err);
  }
  return err;
}

int main(void) {
  LOG_INF("ANT+ Bicycle Power RX sample started.");

  int err = ant_stack_setup();
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

  return 0;

ERROR_EXIT:
  ant_state_indicator_fatal_error();
  k_oops();

  return 0;
}
