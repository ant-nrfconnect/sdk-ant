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

#include <ant_key_manager.h>
#include <ant_parameters.h>
#include <ant_profiles/bpwr/simulator/ant_bpwr_simulator.h>
#include <ant_state_indicator.h>
#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(bpwr_tx, LOG_LEVEL_INF);

#define TIMER_TICK_MS (ANT_HRM_OPERATING_TIME_UNIT * 1000)

#define BUTTON_DECREASE DK_BTN1_MSK
#define BUTTON_INCREASE DK_BTN2_MSK
#define CALIBRATION_RESPONSE DK_BTN3_MSK

void ant_bpwr_evt_handler(ant_bpwr_profile_t *p_profile, ant_bpwr_evt_t event);
void ant_bpwr_calib_handler(ant_bpwr_profile_t *p_profile, ant_bpwr_page1_data_t *p_page1);

BPWR_SENS_CHANNEL_CONFIG_DEF(bpwr, CONFIG_BPWR_TX_CHANNEL_NUM, CONFIG_BPWR_TX_CHAN_ID_TRANS_TYPE,
                             CONFIG_BPWR_TX_CHAN_ID_DEV_NUM, CONFIG_BPWR_TX_NETWORK_NUM);
BPWR_SENS_PROFILE_CONFIG_DEF(bpwr, (ant_bpwr_torque_t)(CONFIG_SENSOR_TYPE), ant_bpwr_calib_handler,
                             ant_bpwr_evt_handler);

static ant_bpwr_profile_t bpwr;
static ant_bpwr_simulator_t bpwr_simulator;

void ant_bpwr_evt_handler(ant_bpwr_profile_t *p_profile, ant_bpwr_evt_t event) {
  switch (event) {
    case ANT_BPWR_PAGE_1_UPDATED:
      /* fall through */
    case ANT_BPWR_PAGE_16_UPDATED:
      /* fall through */
    case ANT_BPWR_PAGE_17_UPDATED:
      /* fall through */
    case ANT_BPWR_PAGE_18_UPDATED:
      /* fall through */
    case ANT_BPWR_PAGE_80_UPDATED:
      /* fall through */
    case ANT_BPWR_PAGE_81_UPDATED:
      ant_bpwr_simulator_one_iteration(&bpwr_simulator, event);
      break;

    default:
      break;
  }
}

void ant_bpwr_calib_handler(ant_bpwr_profile_t *p_profile, ant_bpwr_page1_data_t *p_page1) {
  switch (p_page1->calibration_id) {
    case ANT_BPWR_CALIB_ID_MANUAL:
      bpwr.BPWR_PROFILE_calibration_id = ANT_BPWR_CALIB_ID_MANUAL_SUCCESS;
      bpwr.BPWR_PROFILE_general_calib_data = CONFIG_BPWR_TX_CALIBRATION_DATA;
      break;

    case ANT_BPWR_CALIB_ID_AUTO:
      bpwr.BPWR_PROFILE_calibration_id = ANT_BPWR_CALIB_ID_MANUAL_SUCCESS;
      bpwr.BPWR_PROFILE_auto_zero_status = p_page1->auto_zero_status;
      bpwr.BPWR_PROFILE_general_calib_data = CONFIG_BPWR_TX_CALIBRATION_DATA;
      break;

    case ANT_BPWR_CALIB_ID_CUSTOM_REQ:
      bpwr.BPWR_PROFILE_calibration_id = ANT_BPWR_CALIB_ID_CUSTOM_REQ_SUCCESS;
      memcpy(bpwr.BPWR_PROFILE_custom_calib_data, p_page1->data.custom_calib,
             sizeof(bpwr.BPWR_PROFILE_custom_calib_data));
      break;

    case ANT_BPWR_CALIB_ID_CUSTOM_UPDATE:
      bpwr.BPWR_PROFILE_calibration_id = ANT_BPWR_CALIB_ID_CUSTOM_UPDATE_SUCCESS;
      memcpy(bpwr.BPWR_PROFILE_custom_calib_data, p_page1->data.custom_calib,
             sizeof(bpwr.BPWR_PROFILE_custom_calib_data));
      break;

    default:
      break;
  }
}

void simulator_setup(void) {
  const ant_bpwr_simulator_cfg_t simulator_cfg = {
      .p_profile = &bpwr,
      .sensor_type = (ant_bpwr_torque_t)(CONFIG_SENSOR_TYPE),
  };

#if CONFIG_UPDATE_AUTO
  ant_bpwr_simulator_init(&bpwr_simulator, &simulator_cfg, true);
#else
  ant_bpwr_simulator_init(&bpwr_simulator, &simulator_cfg, false);
#endif
}

static void button_changed(uint32_t button_state, uint32_t has_changed) {
#if CONFIG_UPDATE_MANUAL
  if (has_changed & BUTTON_INCREASE) {
    if (button_state & BUTTON_INCREASE) {
      ant_bpwr_simulator_increment(&bpwr_simulator);
    }
  }
  if (has_changed & BUTTON_DECREASE) {
    if (button_state & BUTTON_DECREASE) {
      ant_bpwr_simulator_decrement(&bpwr_simulator);
    }
  }
#endif
  if (has_changed & CALIBRATION_RESPONSE) {
    if (button_state & CALIBRATION_RESPONSE) {
      ant_bpwr_calib_response(&bpwr);
    }
  }
}

static int utils_setup(void) {
  int err = ant_state_indicator_init(bpwr.channel_number, BPWR_SENS_CHANNEL_TYPE);
  if (err) {
    LOG_ERR("ant_state_indicator_init failed: %d", err);
    return err;
  }

  err = dk_buttons_init(button_changed);
  if (err) {
    LOG_ERR("Cannot init buttons (err: %d)", err);
    return err;
  }

  return err;
}

static void ant_evt_handler(ant_evt_t *p_ant_evt) { ant_bpwr_sens_evt_handler(p_ant_evt, &bpwr); }

static int profile_setup(void) {
  int err =
      ant_bpwr_sens_init(&bpwr, BPWR_SENS_CHANNEL_CONFIG(bpwr), BPWR_SENS_PROFILE_CONFIG(bpwr));
  if (err) {
    LOG_ERR("ant_bpwr_sens_init failed: %d", err);
    return err;
  }

  // fill manufacturer's common data page.
  bpwr.page_80 =
      ANT_COMMON_page80(CONFIG_BPWR_TX_HW_VERSION, CONFIG_BPWR_TX_MFG_ID, CONFIG_BPWR_TX_MODEL_NUM);
  // fill product's common data page.
  bpwr.page_81 = ANT_COMMON_page81(CONFIG_BPWR_TX_SW_VERSION, CONFIG_BPWR_TX_SW_VERSION,
                                   CONFIG_BPWR_TX_SERIAL_NUM);

  bpwr.BPWR_PROFILE_auto_zero_status = ANT_BPWR_AUTO_ZERO_OFF;

  err = ant_bpwr_sens_open(&bpwr);
  if (err) {
    LOG_ERR("ant_bpwr_sens_open failed: %d", err);
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

  err = ant_plus_key_set(CONFIG_BPWR_TX_NETWORK_NUM);
  if (err) {
    LOG_ERR("ant_plus_key_set failed: %d", err);
  }
  return err;
}

int main(void) {
  LOG_INF("ANT+ Bicycle Power TX sample started.");

  int err = ant_stack_setup();
  if (err) {
    goto ERROR_EXIT;
  }

  err = utils_setup();
  if (err) {
    goto ERROR_EXIT;
  }

  simulator_setup();

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
