/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>

#include <ant_host_init.h>
#include <ant_state_indicator.h>

#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_state_indicator, LOG_LEVEL_INF);

#define ADVERTISING_LED_ON_INTERVAL_MS              200
#define ADVERTISING_LED_OFF_INTERVAL_MS             1800

#define ADVERTISING_DIRECTED_LED_ON_INTERVAL_MS     200
#define ADVERTISING_DIRECTED_LED_OFF_INTERVAL_MS    200

#define ADVERTISING_WHITELIST_LED_ON_INTERVAL_MS    200
#define ADVERTISING_WHITELIST_LED_OFF_INTERVAL_MS   800

#define ADVERTISING_SLOW_LED_ON_INTERVAL_MS         400
#define ADVERTISING_SLOW_LED_OFF_INTERVAL_MS        4000

#define BONDING_INTERVAL_MS                         100

#define SENT_OK_INTERVAL_MS                         100
#define SEND_ERROR_INTERVAL_MS                      500

#define RCV_OK_INTERVAL_MS                          100
#define RCV_ERROR_INTERVAL_MS                       500

#define ALERT_INTERVAL_MS                           200

#define LED_INDICATE_SENT_OK                        DK_LED2
#define LED_INDICATE_SEND_ERROR                     DK_LED2
#define LED_INDICATE_RCV_OK                         DK_LED2
#define LED_INDICATE_RCV_ERROR                      DK_LED2
#define LED_INDICATE_CONNECTED                      DK_LED1
#define LED_INDICATE_BONDING                        DK_LED1
#define LED_INDICATE_ADVERTISING_DIRECTED           DK_LED1
#define LED_INDICATE_ADVERTISING_SLOW               DK_LED1
#define LED_INDICATE_ADVERTISING_WHITELIST          DK_LED1
#define LED_INDICATE_INDICATE_ADVERTISING           DK_LED1

#define LED_ALERT                                   DK_LED3

typedef enum
{
    INDICATE_FIRST = 0,
    INDICATE_IDLE  = INDICATE_FIRST,     /**< See \ref INDICATE_IDLE.*/
    INDICATE_SCANNING,                   /**< See \ref INDICATE_SCANNING.*/
    INDICATE_ADVERTISING,                /**< See \ref INDICATE_ADVERTISING.*/
    INDICATE_ADVERTISING_WHITELIST,      /**< See \ref INDICATE_ADVERTISING_WHITELIST.*/
    INDICATE_ADVERTISING_SLOW,           /**< See \ref INDICATE_ADVERTISING_SLOW.*/
    INDICATE_ADVERTISING_DIRECTED,       /**< See \ref INDICATE_ADVERTISING_DIRECTED.*/
    INDICATE_BONDING,                    /**< See \ref INDICATE_BONDING.*/
    INDICATE_CONNECTED,                  /**< See \ref INDICATE_CONNECTED.*/
    INDICATE_SENT_OK,                    /**< See \ref INDICATE_SENT_OK.*/
    INDICATE_SEND_ERROR,                 /**< See \ref INDICATE_SEND_ERROR.*/
    INDICATE_RCV_OK,                     /**< See \ref INDICATE_RCV_OK.*/
    INDICATE_RCV_ERROR,                  /**< See \ref INDICATE_RCV_ERROR.*/
    INDICATE_FATAL_ERROR,                /**< See \ref INDICATE_FATAL_ERROR.*/
    INDICATE_ALERT_0,                    /**< See \ref INDICATE_ALERT_0.*/
    INDICATE_ALERT_1,                    /**< See \ref INDICATE_ALERT_1.*/
    INDICATE_ALERT_2,                    /**< See \ref INDICATE_ALERT_2.*/
    INDICATE_ALERT_3,                    /**< See \ref INDICATE_ALERT_3.*/
    INDICATE_ALERT_OFF,                  /**< See \ref INDICATE_ALERT_OFF.*/
    INDICATE_LAST = INDICATE_ALERT_OFF
} indication_t;

static indication_t m_stable_state      = INDICATE_IDLE;
static bool         m_leds_clear        = false;
static bool         m_alert_on          = false;
static uint8_t      m_leds_mask;

static void leds_timeout_handler(struct k_timer *timer_id);
static K_TIMER_DEFINE(leds_timer, leds_timeout_handler, NULL);

static void alert_timeout_handler(struct k_timer *timer_id);
static K_TIMER_DEFINE(alert_timer, alert_timeout_handler, NULL);

static uint8_t m_channel = UINT8_MAX;   /**< ANT channel number linked to indication. */
static uint8_t m_channel_type;          /**< Type of linked ANT channel. */

static void set_all_leds_off(void)
{
  m_leds_mask = 0;
  dk_set_leds(m_leds_mask);
}

static void set_all_leds_on(void)
{
  m_leds_mask = DK_ALL_LEDS_MSK;
  dk_set_leds(m_leds_mask);
}

static void invert_led(uint8_t led_index)
{
  m_leds_mask ^= BIT(led_index);
  dk_set_leds(m_leds_mask);
}

static void set_led_on(uint8_t led_index)
{
  m_leds_mask |= BIT(led_index);
  dk_set_leds(m_leds_mask);
}

static void set_led_off(uint8_t led_index)
{
  m_leds_mask &= ~BIT(led_index);
  dk_set_leds(m_leds_mask);
}

static bool get_led(uint8_t led_index)
{
  return m_leds_mask & BIT(led_index);
}

static void set_indicate(indication_t indicate)
{
    uint32_t next_delay = 0;

    if (m_stable_state == INDICATE_FATAL_ERROR) {
        return;
    }

    if (m_leds_clear) {
        m_leds_clear = false;
        set_all_leds_off();
    }

    switch (indicate) {
        case INDICATE_IDLE:
            set_all_leds_off();
            k_timer_stop(&leds_timer);
            m_stable_state = indicate;
            break;

        case INDICATE_SCANNING:
        case INDICATE_ADVERTISING:
            /* Blink LED */
            if (get_led(LED_INDICATE_INDICATE_ADVERTISING))
            {
                set_led_off(LED_INDICATE_INDICATE_ADVERTISING);
                next_delay = indicate ==
                             INDICATE_ADVERTISING ? ADVERTISING_LED_OFF_INTERVAL_MS :
                             ADVERTISING_SLOW_LED_OFF_INTERVAL_MS;
            }
            else
            {
                set_led_on(LED_INDICATE_INDICATE_ADVERTISING);
                next_delay = indicate ==
                             INDICATE_ADVERTISING ? ADVERTISING_LED_ON_INTERVAL_MS :
                             ADVERTISING_SLOW_LED_ON_INTERVAL_MS;
            }
            m_stable_state = indicate;
            k_timer_start(&leds_timer, K_MSEC(next_delay), K_NO_WAIT);
            break;

        case INDICATE_ADVERTISING_WHITELIST:
            /* Quickly blink LED */
            if (get_led(LED_INDICATE_ADVERTISING_WHITELIST))
            {
                set_led_off(LED_INDICATE_ADVERTISING_WHITELIST);
                next_delay = indicate ==
                             INDICATE_ADVERTISING_WHITELIST ?
                             ADVERTISING_WHITELIST_LED_OFF_INTERVAL_MS :
                             ADVERTISING_SLOW_LED_OFF_INTERVAL_MS;
            }
            else
            {
                set_led_on(LED_INDICATE_ADVERTISING_WHITELIST);
                next_delay = indicate ==
                             INDICATE_ADVERTISING_WHITELIST ?
                             ADVERTISING_WHITELIST_LED_ON_INTERVAL_MS :
                             ADVERTISING_SLOW_LED_ON_INTERVAL_MS;
            }
            m_stable_state = indicate;
            k_timer_start(&leds_timer, K_MSEC(next_delay), K_NO_WAIT);
            break;

        case INDICATE_ADVERTISING_SLOW:
            /* Slowly blink LED */
            if (get_led(LED_INDICATE_ADVERTISING_SLOW))
            {
                set_led_off(LED_INDICATE_ADVERTISING_SLOW);
                next_delay = indicate ==
                             INDICATE_ADVERTISING_SLOW ? ADVERTISING_SLOW_LED_OFF_INTERVAL_MS :
                             ADVERTISING_SLOW_LED_OFF_INTERVAL_MS;
            }
            else
            {
                set_led_on(LED_INDICATE_ADVERTISING_SLOW);
                next_delay = indicate ==
                             INDICATE_ADVERTISING_SLOW ? ADVERTISING_SLOW_LED_ON_INTERVAL_MS :
                             ADVERTISING_SLOW_LED_ON_INTERVAL_MS;
            }
            m_stable_state = indicate;
            k_timer_start(&leds_timer, K_MSEC(next_delay), K_NO_WAIT);
            break;

        case INDICATE_ADVERTISING_DIRECTED:
            /* Very quickly blink LED */
            if (get_led(LED_INDICATE_ADVERTISING_DIRECTED))
            {
                set_led_off(LED_INDICATE_ADVERTISING_DIRECTED);
                next_delay = indicate ==
                             INDICATE_ADVERTISING_DIRECTED ?
                             ADVERTISING_DIRECTED_LED_OFF_INTERVAL_MS :
                             ADVERTISING_SLOW_LED_OFF_INTERVAL_MS;
            }
            else
            {
                set_led_on(LED_INDICATE_ADVERTISING_DIRECTED);
                next_delay = indicate ==
                             INDICATE_ADVERTISING_DIRECTED ?
                             ADVERTISING_DIRECTED_LED_ON_INTERVAL_MS :
                             ADVERTISING_SLOW_LED_ON_INTERVAL_MS;
            }
            m_stable_state = indicate;
            k_timer_start(&leds_timer, K_MSEC(next_delay), K_NO_WAIT);
            break;

        case INDICATE_BONDING:
            /* Fast blink LED */
            invert_led(LED_INDICATE_BONDING);
            m_stable_state = indicate;
            k_timer_start(&leds_timer, K_MSEC(BONDING_INTERVAL_MS), K_NO_WAIT);
            break;

        case INDICATE_CONNECTED:
            set_led_on(LED_INDICATE_CONNECTED);
            m_stable_state = indicate;
            break;

        case INDICATE_SENT_OK:
            /* Shortly invert LED */
            m_leds_clear = true;
            invert_led(LED_INDICATE_SENT_OK);
            k_timer_start(&leds_timer, K_MSEC(SENT_OK_INTERVAL_MS), K_NO_WAIT);
            break;

        case INDICATE_SEND_ERROR:
            /* Invert LED for a long time */
            m_leds_clear = true;
            invert_led(LED_INDICATE_SEND_ERROR);
            k_timer_start(&leds_timer, K_MSEC(SEND_ERROR_INTERVAL_MS), K_NO_WAIT);
            break;

        case INDICATE_RCV_OK:
            /* Shortly invert LED */
            m_leds_clear = true;
            invert_led(LED_INDICATE_RCV_OK);
            k_timer_start(&leds_timer, K_MSEC(RCV_OK_INTERVAL_MS), K_NO_WAIT);
            break;

        case INDICATE_RCV_ERROR:
            /* Invert LED for long time */
            m_leds_clear = true;
            invert_led(LED_INDICATE_RCV_ERROR);
            k_timer_start(&leds_timer, K_MSEC(RCV_ERROR_INTERVAL_MS), K_NO_WAIT);
            break;

        case INDICATE_FATAL_ERROR:
            /* Turn on all LEDs */
            set_all_leds_on();
            m_stable_state = indicate;
            break;

        case INDICATE_ALERT_0:
            /* fall through */
        case INDICATE_ALERT_1:
            /* fall through */
        case INDICATE_ALERT_2:
            /* fall through */
        case INDICATE_ALERT_3:
            /* fall through */
        case INDICATE_ALERT_OFF:
            k_timer_stop(&alert_timer);
            next_delay = (uint32_t)INDICATE_ALERT_OFF - (uint32_t)indicate;
            if (next_delay)
            {
                if (next_delay > 1)
                {
                  int period = (uint16_t)next_delay * ALERT_INTERVAL_MS;
                  k_timer_start(&alert_timer, K_MSEC(period), K_MSEC(period));
                }
                set_led_on(LED_ALERT);
                m_alert_on = true;
            }
            else
            {
                set_led_off(LED_ALERT);
                m_alert_on = false;
            }
            break;
        default:
            break;
    }
}

/**
 * @brief Function for handling ANT events.
 *
 * @param[in]   p_ant_evt       Event received from the ANT stack.
 */
static void ant_evt_handler(ant_evt_t * p_ant_evt)
{
  if (m_channel != p_ant_evt->channel)
  {
    return;
  }

  switch (m_channel_type)
  {
    case CHANNEL_TYPE_SLAVE:
      /* fall through */
    case CHANNEL_TYPE_SLAVE_RX_ONLY:
      switch (p_ant_evt->event) {
        case EVENT_RX:
          set_indicate(INDICATE_CONNECTED);
          break;
        case EVENT_RX_FAIL:
          set_indicate(INDICATE_RCV_ERROR);
          break;
        case EVENT_RX_FAIL_GO_TO_SEARCH:
          set_indicate(INDICATE_SCANNING);
          break;
        case EVENT_CHANNEL_CLOSED:
          set_indicate(INDICATE_IDLE);
          break;
        case EVENT_RX_SEARCH_TIMEOUT:
          set_indicate(INDICATE_IDLE);
          break;
      }
      break;
  }
}

static void leds_timeout_handler(struct k_timer *timer_id)
{
  set_indicate(m_stable_state);
}

static void alert_timeout_handler(struct k_timer *timer_id)
{
  invert_led(LED_ALERT);
}

int ant_state_indicator_init( uint8_t channel, uint8_t channel_type)
{
  m_channel       = channel;
  m_channel_type  = channel_type;
  m_stable_state  = INDICATE_IDLE;

  int err = ant_cb_register(&ant_evt_handler);
  if (err) {
    LOG_ERR("ant_cb_register failed: %d", err);
    return err;
  }

  err = dk_leds_init();
  if (err) {
    LOG_ERR("LEDs init failed (err %d)", err);
    return err;
  }

  m_leds_mask = 0;
  set_indicate(INDICATE_IDLE);

  return 0;
}

void ant_state_indicator_channel_opened(void)
{
  switch (m_channel_type)
  {
    case CHANNEL_TYPE_SLAVE:
      /* fall through */
    case CHANNEL_TYPE_SLAVE_RX_ONLY:
      set_indicate(INDICATE_SCANNING);
      break;
    case CHANNEL_TYPE_MASTER:
      set_indicate(INDICATE_ADVERTISING);
      break;
    default:
      break;
  }
}

void ant_state_indicator_fatal_error(void)
{
  set_indicate(INDICATE_FATAL_ERROR);
}
