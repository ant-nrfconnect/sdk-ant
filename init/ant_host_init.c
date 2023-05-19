/*
 * Copyright 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include <zephyr/init.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>

#include "ant_host_init.h"
#include "ant_interface.h"

#if !defined(CONFIG_ANT_NP_HOST_SYS_INIT)
#include "ant_rpc_app.h"
#endif // !CONFIG_ANT_NP_HOST_SYS_INIT

static struct k_work ant_work;
struct k_work_q ant_work_q;
static K_THREAD_STACK_DEFINE(ant_work_stack, CONFIG_ANT_WORK_STACK_SIZE);

LOG_MODULE_REGISTER(ant_host_init, CONFIG_ANT_LOG_LEVEL);

K_SEM_DEFINE(ant_event_sem, 0, 1);

static sys_slist_t ant_callbacks = SYS_SLIST_STATIC_INIT(&ant_callbacks);
static struct k_mutex ant_callbacks_mut;

struct ant_evt_cb_entry {
   sys_snode_t node;
   ant_evt_callback_t handler;
};

ant_err_t ant_init(void)
{
   // Note: ANT stack initializations of resources for configured # channels, buffers.. etc performed by cpunet core
   //       cpuapp core does not initiate this

#if !defined(CONFIG_ANT_NP_HOST_SYS_INIT)
   // If Kconfig has been used to delay RPC init via SYS_INIT, complete the initialization here
   ant_err_t err = ant_rpc_app_init();
   if (err) {
      LOG_ERR("ant_rpc_app_init() failed: %d", err);
      return err;
   }
#endif // !CONFIG_ANT_NP_HOST_SYS_INIT

   return 0;
}

ant_err_t ant_cb_register(ant_evt_callback_t evt_handler)
{
   if (evt_handler == NULL)
   {
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

extern ant_err_t ant_np_host_cmd_passthrough(ANT_MESSAGE *cmd, ANT_MESSAGE *rsp);
ant_err_t ant_cmd_passthrough(ANT_MESSAGE *cmd, ANT_MESSAGE *rsp)
{
   return (ant_np_host_cmd_passthrough(cmd, rsp));
}

static void ant_work_handler(struct k_work *item)
{
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

static int ant_thread_init(void)
{
   k_work_queue_start(&ant_work_q, ant_work_stack,
                  K_THREAD_STACK_SIZEOF(ant_work_stack),
                  K_PRIO_COOP(CONFIG_ANT_THREAD_COOP_PRIO), NULL);
   k_thread_name_set(&ant_work_q.thread, "ANT Host Work");
   k_work_init(&ant_work, ant_work_handler);

   return 0;
}

void ant_host_rx_thread(void)
{
   while (1) {
      /* Wait for event */
      k_sem_take(&ant_event_sem, K_FOREVER);

      k_work_submit_to_queue(&ant_work_q, &ant_work);
   }
}

SYS_INIT(ant_thread_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

K_THREAD_DEFINE(ant_host_rx_thread_id, CONFIG_ANT_WORK_STACK_SIZE, ant_host_rx_thread, NULL, NULL,
      NULL, K_PRIO_COOP(CONFIG_ANT_THREAD_COOP_PRIO), 0, 0);
