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

#include <ant_parameters.h>
#include <ant_state_indicator.h>
#include <ant_key_manager.h>
#include <ant_profiles/hrm/simulator/ant_hrm_simulator.h>

#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(hrm_tx, LOG_LEVEL_INF);

#define TIMER_TICK_MS       (ANT_HRM_OPERATING_TIME_UNIT * 1000)

#define BUTTON_DECREASE     DK_BTN1_MSK
#define BUTTON_INCREASE     DK_BTN2_MSK

static void ant_hrm_evt_handler(ant_hrm_profile_t * p_profile, ant_hrm_evt_t event);

HRM_SENS_PROFILE_CONFIG_DEF(hrm, true, ANT_HRM_PAGE_0, ant_hrm_evt_handler);
HRM_SENS_CHANNEL_CONFIG_DEF(hrm,
  CONFIG_HRM_TX_CHANNEL_NUM,
  CONFIG_HRM_TX_CHAN_ID_TRANS_TYPE,
  CONFIG_HRM_TX_CHAN_ID_DEV_NUM,
  CONFIG_HRM_TX_NETWORK_NUM);

static void timeout_handler(struct k_timer *timer_id);
static K_TIMER_DEFINE(timer, timeout_handler, NULL);

static ant_hrm_profile_t hrm;
static ant_hrm_simulator_t hrm_simulator;

static void ant_hrm_evt_handler(ant_hrm_profile_t * p_profile, ant_hrm_evt_t event)
{
  switch (event) {
    case ANT_HRM_PAGE_0_UPDATED:
      /* fall through */
    case ANT_HRM_PAGE_1_UPDATED:
      /* fall through */
    case ANT_HRM_PAGE_2_UPDATED:
      /* fall through */
    case ANT_HRM_PAGE_3_UPDATED:
      /* fall through */
    case ANT_HRM_PAGE_4_UPDATED:
      ant_hrm_simulator_one_iteration(&hrm_simulator);
      break;
    default:
      break;
  }
}

static void simulator_setup(void)
{
  const ant_hrm_simulator_cfg_t simulator_cfg = DEFAULT_ANT_HRM_SIMULATOR_CFG(&hrm,
    CONFIG_SIMULATOR_MIN,
    CONFIG_SIMULATOR_MAX,
    CONFIG_SIMULATOR_INCREMENT);

#if CONFIG_UPDATE_AUTO
  ant_hrm_simulator_init(&hrm_simulator, &simulator_cfg, true);
#else
  ant_hrm_simulator_init(&hrm_simulator, &simulator_cfg, false);
#endif
}

#if CONFIG_UPDATE_MANUAL
static void button_changed(uint32_t button_state, uint32_t has_changed)
{
  if (has_changed & BUTTON_INCREASE) {
    if (button_state & BUTTON_INCREASE) {
      LOG_INF("Increase heart rate!");
      ant_hrm_simulator_increment(&hrm_simulator);
    }
  }
  if (has_changed & BUTTON_DECREASE) {
    if (button_state & BUTTON_DECREASE) {
      LOG_INF("Decrease heart rate!");
      ant_hrm_simulator_decrement(&hrm_simulator);
    }
  }
}
#endif

static void timeout_handler(struct k_timer *timer_id)
{
  /** NOTE: Only the first 3 bytes of this value are taken into account. */
  hrm.HRM_PROFILE_operating_time++;
}

static int utils_setup(void)
{
  int err = ant_state_indicator_init(hrm.channel_number, HRM_SENS_CHANNEL_TYPE);
  if (err) {
    LOG_ERR("ant_state_indicator_init failed: %d", err);
    return err;
  }

#if CONFIG_UPDATE_MANUAL
  err = dk_buttons_init(button_changed);
  if (err) {
    LOG_ERR("Cannot init buttons (err: %d)", err);
    return err;
  }
#endif

  return err;
}

static void ant_evt_handler(ant_evt_t *p_ant_evt)
{
  ant_hrm_sens_evt_handler(p_ant_evt, &hrm);
}

static int profile_setup(void)
{
  int err = ant_hrm_sens_init(&hrm,
    HRM_SENS_CHANNEL_CONFIG(hrm),
    HRM_SENS_PROFILE_CONFIG(hrm));
  if (err) {
    LOG_ERR("ant_hrm_sens_init failed: %d", err);
    return err;
  }

  hrm.HRM_PROFILE_manuf_id   = CONFIG_HRM_TX_MFG_ID;
  hrm.HRM_PROFILE_serial_num = CONFIG_HRM_TX_SERIAL_NUM;
  hrm.HRM_PROFILE_hw_version = CONFIG_HRM_TX_HW_VERSION;
  hrm.HRM_PROFILE_sw_version = CONFIG_HRM_TX_SW_VERSION;
  hrm.HRM_PROFILE_model_num  = CONFIG_HRM_TX_MODEL_NUM;

  err = ant_hrm_sens_open(&hrm);
  if (err) {
    LOG_ERR("ant_hrm_sens_open failed: %d", err);
    return err;
  }
  ant_state_indicator_channel_opened();
  return err;
}

int ant_stack_setup(void)
{
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

  err = ant_plus_key_set(CONFIG_HRM_TX_NETWORK_NUM);
  if (err) {
    LOG_ERR("ant_plus_key_set failed: %d", err);
  }
  return err;
}

int main(void)
{
  LOG_INF("ANT+ HRM TX sample starting...");

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

  k_timer_start(&timer, K_MSEC(TIMER_TICK_MS), K_MSEC(TIMER_TICK_MS));
  return 0;

ERROR_EXIT:
  ant_state_indicator_fatal_error();
  k_oops();

  return 0;
}
