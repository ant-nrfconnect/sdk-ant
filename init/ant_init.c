/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include <zephyr/init.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>
#include <errno.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>

#include "ant_init.h"
#include "ant_interface.h"

static struct k_work ant_work;
struct k_work_q ant_work_q;
static K_THREAD_STACK_DEFINE(ant_work_stack, CONFIG_ANT_WORK_STACK_SIZE);

#define ANT_EVT_PRIORITY (4)

LOG_MODULE_REGISTER(ant_init, CONFIG_ANT_LOG_LEVEL);

#if defined(CONFIG_ANT_SWI1)
#define ANT_SWI_IRQN SWI1_IRQn
#elif defined(CONFIG_ANT_SWI2)
#define ANT_SWI_IRQN SWI2_IRQn
#elif defined(CONFIG_ANT_SWI3)
#define ANT_SWI_IRQN SWI3_IRQn
#else
#error "Software interrupt for ANT events has not been defined."
#endif

IRQn_Type const ant_evt_irqn = ANT_SWI_IRQN;

static ant_evt_callback_t ant_cb;

// Memory buffer provided in order to support channel configuration.
__ALIGN(4) static uint8_t m_ant_stack_buffer[NRF_ANT_BUF_SIZE];

ant_err_t ant_init(void) {
  ant_err_t err;

  ANT_ENABLE ant_enable_cfg = {
      .ucTotalNumberOfChannels = CONFIG_ANT_TOTAL_CHANNELS_ALLOCATED,
      .ucNumberOfEncryptedChannels = CONFIG_ANT_ENCRYPTED_CHANNELS,
      .usNumberOfEvents = CONFIG_ANT_EVENT_QUEUE_SIZE,
      .pucMemoryBlockStartLocation = m_ant_stack_buffer,
      .usMemoryBlockByteSize = sizeof(m_ant_stack_buffer),
  };

  // ant init & enable stack
  err = ant_stack_init(CONFIG_ANT_LICENSE_KEY);
  if (!err) {
    err = ant_enable(&ant_enable_cfg);
  }

  return err;
}

ant_err_t ant_cb_register(ant_evt_callback_t evt_handler) {
  if (evt_handler == NULL) {
    return -EINVAL;
  }
  ant_cb = evt_handler;

  return 0;
}

static int ant_lib_init(const struct device *dev) {
  ARG_UNUSED(dev);

  irq_disable(ant_evt_irqn);

  BUILD_ASSERT(
      1 != sizeof(CONFIG_ANT_LICENSE_KEY),
      "You must obtain a valid license key to use ANT. You may use the evaluation key for non-"
      "commercial use only with CONFIG_ANT_EVALUATION_KEY=y set on the network core. Commercial use "
      "license keys are available from ANT Wireless, see CONFIG_ANT_LICENSE_KEY for details.");

  ant_err_t err = ant_stack_init(CONFIG_ANT_LICENSE_KEY);
  if (err) {
    LOG_ERR("ant_stack_init() failed: %d", err);
    return -EINVAL;
  }

  return 0;
}

static void ant_evt_irq_handler(const void *arg) { k_work_submit_to_queue(&ant_work_q, &ant_work); }

static void ant_work_handler(struct k_work *item) {
  ARG_UNUSED(item);

  ant_err_t err;
  ant_evt_t ant_evt;

  do {
    ant_evt.event = NO_EVENT;
    err = ant_event_get(&ant_evt.channel, &ant_evt.event, ant_evt.message.aucMessage);
    if (err && (err != -ENOENT)) {
      LOG_ERR("ant_event_get() failed: %d", err);
    } else {
      // valid event ( != NO_EVENT )
      if (ant_cb && ant_evt.event) {
        ant_cb(&ant_evt);
      }
    }
  } while (ant_evt.event);
}

static int ant_thread_init(const struct device *dev) {
  ARG_UNUSED(dev);

  k_work_queue_start(&ant_work_q, ant_work_stack, K_THREAD_STACK_SIZEOF(ant_work_stack),
                     K_PRIO_COOP(CONFIG_ANT_THREAD_COOP_PRIO), NULL);
  k_thread_name_set(&ant_work_q.thread, "ANT Work");
  k_work_init(&ant_work, ant_work_handler);

  IRQ_CONNECT(ant_evt_irqn, ANT_EVT_PRIORITY, ant_evt_irq_handler, NULL, 0);
  irq_enable(ant_evt_irqn);

  return 0;
}

SYS_INIT(ant_lib_init, PRE_KERNEL_2, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
SYS_INIT(ant_thread_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
