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

#include <ant_parameters.h>
#include <ant_state_indicator.h>
#include <ant_key_manager.h>
#include <ant_profiles/ant_hrm.h>

#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(hrm_rx, LOG_LEVEL_INF);

static void ant_hrm_evt_handler(ant_hrm_profile_t * p_profile, ant_hrm_evt_t event);

HRM_DISP_CHANNEL_CONFIG_DEF(hrm,
  CONFIG_HRM_RX_CHANNEL_NUM,
  CONFIG_HRM_RX_CHAN_ID_TRANS_TYPE,
  CONFIG_HRM_RX_CHAN_ID_DEV_NUM,
  CONFIG_HRM_RX_NETWORK_NUM,
  HRM_MSG_PERIOD_4Hz);

static ant_hrm_profile_t hrm;

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
      LOG_INF("Page was updated");
      break;
    default:
      break;
  }
}

static int utils_setup(void)
{
  int err = ant_state_indicator_init(hrm.channel_number, HRM_DISP_CHANNEL_TYPE);
  if (err) {
    LOG_ERR("ant_state_indicator_init failed: %d", err);
    return err;
  }

  return err;
}

static void ant_evt_handler(ant_evt_t *p_ant_evt)
{
  ant_hrm_disp_evt_handler(p_ant_evt, &hrm);
}

static int profile_setup(void)
{
  int err = ant_hrm_disp_init(&hrm,
    HRM_DISP_CHANNEL_CONFIG(hrm),
    ant_hrm_evt_handler);
  if (err) {
    LOG_ERR("ant_hrm_disp_init failed: %d", err);
    return err;
  }

  err = ant_hrm_disp_open(&hrm);
  if (err) {
    LOG_ERR("ant_hrm_disp_open failed: %d", err);
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

  err = ant_plus_key_set(CONFIG_HRM_RX_NETWORK_NUM);
  if (err) {
    LOG_ERR("ant_plus_key_set failed: %d", err);
  }
  return err;
}

void main(void)
{
  LOG_INF("ANT+ HRM RX sample starting...");

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

  return;

ERROR_EXIT:
  ant_state_indicator_fatal_error();
  k_oops();
}
