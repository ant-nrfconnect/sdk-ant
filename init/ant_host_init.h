/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_HOST_INIT_H__
#define ANT_HOST_INIT_H__

#include "ant_error.h"
#include "ant_parameters.h"

#include <stdint.h>

#define NRF_ANT_EVT_CHANNEL_FIELD_SIZE 1  //!< Size of the channel field in ANT stack event
#define NRF_ANT_EVT_EVENT_FIELD_SIZE 1    //!< Size of the event field in ANT stack event

/**@brief Size of the buffer provided to the ANT stack to receive ANT events. */
#define NRF_ANT_MESSAGE_SIZE ((CEIL_DIV(MESG_BUFFER_SIZE, sizeof(uint32_t))) * sizeof(uint32_t))

/**@brief Size of the buffer provided to the Events Scheduler to hold ANT events. */
#define NRF_ANT_EVT_BUF_SIZE                                                                       \
  ((CEIL_DIV(NRF_ANT_MESSAGE_SIZE + NRF_ANT_EVT_CHANNEL_FIELD_SIZE + NRF_ANT_EVT_EVENT_FIELD_SIZE, \
             sizeof(uint32_t))) *                                                                  \
   sizeof(uint32_t))

/**@struct ant_evt_t ant_host_init.h
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

typedef void (*ant_evt_callback_t)(ant_evt_t *p_ant_evt);

/* @brief Function for handling host side ANT initializations */
ant_err_t ant_init(void);

/* @brief Function for registration of callback(s) to handle received ANT events */
ant_err_t ant_cb_register(ant_evt_callback_t evt_handler);

/**@brief Function for passing in ANT_MESSAGE cmds from host to remote. Provided rsp
 *        input populated with ANT_MESSAGE response to cmd. Intended for apps dealing
 *        with raw cmds/rsps in ANT_MESSAGE format and forgoes usage of issuance of
 *        ANT cmd via apis in @ref ant_interface.h */
ant_err_t ant_cmd_passthrough(ANT_MESSAGE *cmd, ANT_MESSAGE *rsp);
#endif  // ANT_HOST_INIT_H__
