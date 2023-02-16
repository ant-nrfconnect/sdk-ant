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

#include <ant_parameters.h>
#include <ant_state_indicator.h>
#include <ant_key_manager.h>
#include <ant_profiles/bsc/ant_bsc.h>

#include <dk_buttons_and_leds.h>

#define WHEEL_CIRCUMFERENCE 2070 /**< Bike wheel circumference [mm] */
#define BSC_EVT_TIME_FACTOR 1024 /**< Time unit factor for BSC events */
#define BSC_RPM_TIME_FACTOR 60   /**< Time unit factor for RPM unit */
#define BSC_MS_TO_KPH_NUM 36     /**< Numerator of [m/s] to [kph] ratio */
#define BSC_MS_TO_KPH_DEN 10     /**< Denominator of [m/s] to [kph] ratio */
#define BSC_MM_TO_M_FACTOR 1000  /**< Unit factor [m/s] to [mm/s] */
#define SPEED_COEFFICIENT                                                              \
  (WHEEL_CIRCUMFERENCE * BSC_EVT_TIME_FACTOR * BSC_MS_TO_KPH_NUM / BSC_MS_TO_KPH_DEN / \
   BSC_MM_TO_M_FACTOR) /**< Coefficient for speed value calculation */
#define CADENCE_COEFFICIENT \
  (BSC_EVT_TIME_FACTOR * BSC_RPM_TIME_FACTOR) /**< Coefficient for cadence value calculation */

typedef struct {
  int32_t acc_rev_cnt;
  int32_t prev_rev_cnt;
  int32_t prev_acc_rev_cnt;
  int32_t acc_evt_time;
  int32_t prev_evt_time;
  int32_t prev_acc_evt_time;
} bsc_disp_calc_data_t;

static bsc_disp_calc_data_t m_speed_calc_data = {0};
static bsc_disp_calc_data_t m_cadence_calc_data = {0};

LOG_MODULE_REGISTER(bsc_rx, LOG_LEVEL_INF);

void ant_bsc_evt_handler(ant_bsc_profile_t *p_profile, ant_bsc_evt_t event);

BSC_DISP_CHANNEL_CONFIG_DEF(bsc, CONFIG_BSC_RX_CHANNEL_NUM, CONFIG_BSC_RX_CHAN_ID_TRANS_TYPE,
                            CONFIG_BSC_RX_DISPLAY_TYPE, CONFIG_BSC_RX_CHAN_ID_DEV_NUM,
                            CONFIG_BSC_RX_NETWORK_NUM, BSC_MSG_PERIOD_4Hz);
BSC_DISP_PROFILE_CONFIG_DEF(bsc, ant_bsc_evt_handler);

static ant_bsc_profile_t bsc;

static uint32_t calculate_speed(int32_t rev_cnt, int32_t evt_time) {
  static uint32_t computed_speed = 0;

  if (rev_cnt != m_speed_calc_data.prev_rev_cnt) {
    m_speed_calc_data.acc_rev_cnt += rev_cnt - m_speed_calc_data.prev_rev_cnt;
    m_speed_calc_data.acc_evt_time += evt_time - m_speed_calc_data.prev_evt_time;

    /* Process rollover */
    if (m_speed_calc_data.prev_rev_cnt > rev_cnt) {
      m_speed_calc_data.acc_rev_cnt += UINT16_MAX + 1;
    }
    if (m_speed_calc_data.prev_evt_time > evt_time) {
      m_speed_calc_data.acc_evt_time += UINT16_MAX + 1;
    }

    m_speed_calc_data.prev_rev_cnt = rev_cnt;
    m_speed_calc_data.prev_evt_time = evt_time;

    computed_speed = SPEED_COEFFICIENT *
                     (m_speed_calc_data.acc_rev_cnt - m_speed_calc_data.prev_acc_rev_cnt) /
                     (m_speed_calc_data.acc_evt_time - m_speed_calc_data.prev_acc_evt_time);

    m_speed_calc_data.prev_acc_rev_cnt = m_speed_calc_data.acc_rev_cnt;
    m_speed_calc_data.prev_acc_evt_time = m_speed_calc_data.acc_evt_time;
  }

  return (uint32_t)computed_speed;
}

static uint32_t calculate_cadence(int32_t rev_cnt, int32_t evt_time) {
  static uint32_t computed_cadence = 0;

  if (rev_cnt != m_cadence_calc_data.prev_rev_cnt) {
    m_cadence_calc_data.acc_rev_cnt += rev_cnt - m_cadence_calc_data.prev_rev_cnt;
    m_cadence_calc_data.acc_evt_time += evt_time - m_cadence_calc_data.prev_evt_time;

    /* Process rollover */
    if (m_cadence_calc_data.prev_rev_cnt > rev_cnt) {
      m_cadence_calc_data.acc_rev_cnt += UINT16_MAX + 1;
    }
    if (m_cadence_calc_data.prev_evt_time > evt_time) {
      m_cadence_calc_data.acc_evt_time += UINT16_MAX + 1;
    }

    m_cadence_calc_data.prev_rev_cnt = rev_cnt;
    m_cadence_calc_data.prev_evt_time = evt_time;

    computed_cadence = CADENCE_COEFFICIENT *
                       (m_cadence_calc_data.acc_rev_cnt - m_cadence_calc_data.prev_acc_rev_cnt) /
                       (m_cadence_calc_data.acc_evt_time - m_cadence_calc_data.prev_acc_evt_time);

    m_cadence_calc_data.prev_acc_rev_cnt = m_cadence_calc_data.acc_rev_cnt;
    m_cadence_calc_data.prev_acc_evt_time = m_cadence_calc_data.acc_evt_time;
  }

  return (uint32_t)computed_cadence;
}

static void ant_evt_handler(ant_evt_t *p_ant_evt) {
  ant_bsc_disp_evt_handler(p_ant_evt, &bsc);

  switch (p_ant_evt->event) {
    case EVENT_RX_FAIL_GO_TO_SEARCH:
      /* Reset speed and cadence values */
      memset(&m_speed_calc_data, 0, sizeof(m_speed_calc_data));
      memset(&m_cadence_calc_data, 0, sizeof(m_cadence_calc_data));
      break;

    default:
      /* No implementation needed */
      break;
  }
}

void ant_bsc_evt_handler(ant_bsc_profile_t *p_profile, ant_bsc_evt_t event) {
  switch (event) {
    case ANT_BSC_PAGE_0_UPDATED:
      /* fall through */
    case ANT_BSC_PAGE_1_UPDATED:
      /* fall through */
    case ANT_BSC_PAGE_2_UPDATED:
      /* fall through */
    case ANT_BSC_PAGE_3_UPDATED:
      /* fall through */
    case ANT_BSC_PAGE_4_UPDATED:
      /* fall through */
    case ANT_BSC_PAGE_5_UPDATED:
      /* Log computed value */

      if (CONFIG_BSC_RX_DISPLAY_TYPE == BSC_SPEED_DEVICE_TYPE) {
        LOG_INF("Computed speed value:                 %u kph\n\n",
                (unsigned int)calculate_speed(p_profile->BSC_PROFILE_rev_count,
                                              p_profile->BSC_PROFILE_event_time));
      } else if (CONFIG_BSC_RX_DISPLAY_TYPE == BSC_CADENCE_DEVICE_TYPE) {
        LOG_INF("Computed cadence value:               %u rpm\n\n",
                (unsigned int)calculate_cadence(p_profile->BSC_PROFILE_rev_count,
                                                p_profile->BSC_PROFILE_event_time));
      }
      break;

    case ANT_BSC_COMB_PAGE_0_UPDATED:
      LOG_INF("Computed speed value:                         %u kph",
              (unsigned int)calculate_speed(p_profile->BSC_PROFILE_speed_rev_count,
                                            p_profile->BSC_PROFILE_speed_event_time));
      LOG_INF("Computed cadence value:                       %u rpms\n\n",
              (unsigned int)calculate_cadence(p_profile->BSC_PROFILE_cadence_rev_count,
                                              p_profile->BSC_PROFILE_cadence_event_time));
      break;

    default:
      break;
  }
}

static int utils_setup(void) {
  int err = ant_state_indicator_init(bsc.channel_number, BSC_DISP_CHANNEL_TYPE);

  if (err) {
    LOG_ERR("ant_state_indicator_init failed: %d", err);
  }

  return err;
}

static int profile_setup(void) {
  int err = ant_bsc_disp_init(&bsc, BSC_DISP_CHANNEL_CONFIG(bsc), BSC_DISP_PROFILE_CONFIG(bsc));
  if (err) {
    LOG_ERR("ant_hrm_disp_init failed: %d", err);
    return err;
  }

  err = ant_bsc_disp_open(&bsc);
  if (err) {
    LOG_ERR("ant_hrm_disp_open failed: %d", err);
    return err;
  }

  ant_state_indicator_channel_opened();
  return err;
}

static int ant_stack_setup(void) {
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

  err = ant_plus_key_set(CONFIG_BSC_RX_NETWORK_NUM);
  if (err) {
    LOG_ERR("ant_plus_key_set failed: %d", err);
  }
  return err;
}

void main(void) {
  LOG_INF("ANT+ Bicycle Speed and Cadence RX sample starting...");

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
