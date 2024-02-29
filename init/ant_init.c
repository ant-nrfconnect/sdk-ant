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
#include "ant_parameters.h"

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

static sys_slist_t ant_callbacks = SYS_SLIST_STATIC_INIT(&ant_callbacks);
static struct k_mutex ant_callbacks_mut;

struct ant_evt_cb_entry {
  sys_snode_t node;
  ant_evt_callback_t handler;
};

// Memory buffer provided in order to support channel configuration.
__ALIGN(4) static uint8_t m_ant_stack_buffer[NRF_ANT_BUF_SIZE];

#if (CONFIG_ANT_ENCRYPTED_CHANNELS > 0)
#include <zephyr/drivers/entropy.h>
static const struct device *stEntropySource = DEVICE_DT_GET(DT_NODELABEL(rng));
static ANT_STACK_FUNCS ant_funcs;
static void rand_func(uint8_t *buf, uint8_t len) {
  entropy_get_entropy(stEntropySource, buf, len);
}
#if defined(CONFIG_BT)
#if defined(CONFIG_ANT_SDC_INIT)
static void sdc_assertion_handler(const char *const file, const uint32_t line) {
  LOG_ERR("Softdevice Controller ASSERT: %s, %d", file, line);
  k_oops();
}
#endif // CONFIG_ANT_SDC_INIT
#include <sdc.h>
#include <sdc_soc.h>
static void ecb_encrypt_func(ANT_ECB_DATA *data) {
  // shared access to NRF_ECB using softdevice controller (sdc)
  sdc_soc_ecb_block_encrypt(data->aucKey, data->aucClearText, data->aucCipherText);
}
#else
#include <hal/nrf_ecb.h>
static void ecb_encrypt_func(ANT_ECB_DATA *data) {
  // direct access to NRF_ECB
  nrf_ecb_data_pointer_set(NRF_ECB, data);
  nrf_ecb_event_clear(NRF_ECB, NRF_ECB_EVENT_ENDECB);
  nrf_ecb_event_clear(NRF_ECB, NRF_ECB_EVENT_ERRORECB);
  nrf_ecb_task_trigger(NRF_ECB, NRF_ECB_TASK_STARTECB);
  while (!(nrf_ecb_event_check(NRF_ECB, NRF_ECB_EVENT_ENDECB) ||
          nrf_ecb_event_check(NRF_ECB, NRF_ECB_EVENT_ERRORECB))) {
  }
}
#endif // CONFIG_BT
#endif // CONFIG_ANT_ENCRYPTED_CHANNELS

ant_err_t ant_init(void) {
  ant_err_t err;

  ANT_ENABLE ant_enable_cfg = {
      .ucTotalNumberOfChannels = CONFIG_ANT_TOTAL_CHANNELS_ALLOCATED,
      .ucNumberOfEncryptedChannels = CONFIG_ANT_ENCRYPTED_CHANNELS,
      .usNumberOfEvents = CONFIG_ANT_EVENT_QUEUE_SIZE,
      .pucMemoryBlockStartLocation = m_ant_stack_buffer,
      .usMemoryBlockByteSize = sizeof(m_ant_stack_buffer),
  };

  err = ant_stack_config(&ant_enable_cfg);
  if (err)
    return err;

  // Enable the stack
  err = ant_stack_enable();
  if (err)
    return err;

#if CONFIG_ANT_ENCRYPTED_CHANNELS > 0
#if defined(CONFIG_BT)
#if defined(CONFIG_ANT_SDC_INIT)
  // Allow ANT to initialize the softdevice controller (sdc) for sdc_soc API access. Call after MPSL is init
  err = sdc_init(sdc_assertion_handler);
  if (err)
    return err;
#endif // CONFIG_ANT_SDC_INIT
#endif // CONFIG_BT

  // for encryption support, set RAND & ECB encrypt func
  ant_funcs.fpRANDGet = rand_func;
  ant_funcs.fpECBEncrypt = ecb_encrypt_func;
  err = ant_stack_funcs_register(&ant_funcs);
  if (err)
    return err;

#endif // CONFIG_ANT_ENCRYPTED_CHANNELS

  return err;
}

ant_err_t ant_cb_register(ant_evt_callback_t evt_handler) {
  if (evt_handler == NULL) {
    return -EINVAL;
  }

  struct ant_evt_cb_entry *cb;

  // check if handler is already registered
  k_mutex_lock(&ant_callbacks_mut, K_FOREVER);
  SYS_SLIST_FOR_EACH_CONTAINER(&ant_callbacks, cb, node) {
    if (cb->handler == evt_handler) {
      LOG_DBG("handler is registered, nothing to do");
      k_mutex_unlock(&ant_callbacks_mut);
      return 0;
    }
  }

  // allocate memory for the callback entry
  cb = (struct ant_evt_cb_entry *)k_malloc(sizeof(struct ant_evt_cb_entry));
  if (cb == NULL) {
    k_mutex_unlock(&ant_callbacks_mut);
    return -ENOBUFS;
  }
  memset(cb, 0, sizeof(struct ant_evt_cb_entry));
  cb->handler = evt_handler;

  // insert the handler in the list
  sys_slist_append(&ant_callbacks, &cb->node);
  k_mutex_unlock(&ant_callbacks_mut);
  return 0;
}

static int ant_lib_init(void) {
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

  ant_evt_t ant_evt;

  do {
    ant_evt.event = NO_EVENT;
    ant_event_get(&ant_evt.channel, &ant_evt.event, ant_evt.message.aucMessage);

    // valid event ( != NO_EVENT )
    if (ant_evt.event) {
      struct ant_evt_cb_entry *curr, *tmp;

      if (sys_slist_is_empty(&ant_callbacks)) {
        return;
      }

      k_mutex_lock(&ant_callbacks_mut, K_FOREVER);

      // dispatch events to registered callbacks
      LOG_DBG("dispatching events:");
      SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&ant_callbacks, curr, tmp, node) {
        LOG_DBG(" - handler=0x%08X", (uint32_t)curr->handler);
        curr->handler(&ant_evt);
      }

      k_mutex_unlock(&ant_callbacks_mut);
    }
  } while (ant_evt.event);

  return;
}

static int ant_thread_init(void) {
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
