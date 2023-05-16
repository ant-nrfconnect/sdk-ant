/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_INIT_H__
#define ANT_INIT_H__

#include "ant_error.h"
#include "ant_parameters.h"

#include <zephyr/irq.h>
#include <stdint.h>

#define NRF_ANT_EVT_CHANNEL_FIELD_SIZE 1  //!< Size of the channel field in ANT stack event
#define NRF_ANT_EVT_EVENT_FIELD_SIZE 1    //!< Size of the event field in ANT stack event

/**@brief Size of the buffer provided to the ANT stack.
 * @hideinitializer
 */
#define NRF_ANT_BUF_SIZE                                                                    \
  ANT_ENABLE_GET_REQUIRED_SPACE(CONFIG_ANT_TOTAL_CHANNELS_ALLOCATED,                        \
                                CONFIG_ANT_ENCRYPTED_CHANNELS, CONFIG_ANT_BURST_QUEUE_SIZE, \
                                CONFIG_ANT_EVENT_QUEUE_SIZE)

/**@brief Size of the buffer provided to the ANT stack to receive ANT events. */
#define NRF_ANT_MESSAGE_SIZE ((CEIL_DIV(MESG_BUFFER_SIZE, sizeof(uint32_t))) * sizeof(uint32_t))

/**@brief Size of the buffer provided to the Events Scheduler to hold ANT events. */
#define NRF_ANT_EVT_BUF_SIZE ((CEIL_DIV(NRF_ANT_MESSAGE_SIZE +           \
                                     NRF_ANT_EVT_CHANNEL_FIELD_SIZE +    \
                                     NRF_ANT_EVT_EVENT_FIELD_SIZE,       \
                                     sizeof(uint32_t))) * sizeof(uint32_t))

/**@struct ant_evt_t ant_init.h
 * ANT Stack Event.
 *
 *  @var ant_evt_t::message
 *   ANT Message
 *  @var ant_evt_t::channel
 *   Channel number (including RX burst transfer sequencing if applicable, see "RX Burst Message Sequencing Defines" in @ref ant_parameters.h)
 *  @var ant_evt_t::event
 *    Event code
 */
typedef struct {
  ANT_MESSAGE message;
  uint8_t channel;
  uint8_t event;
} ant_evt_t;

/**@brief External IRQ for events */
extern IRQn_Type const ant_evt_irqn;

typedef void (*ant_evt_callback_t)(ant_evt_t *p_ant_evt);

ant_err_t ant_init(void);
ant_err_t ant_cb_register(ant_evt_callback_t evt_handler);

#endif  // ANT_INIT_H__
