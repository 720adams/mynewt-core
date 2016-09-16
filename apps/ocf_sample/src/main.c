/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <assert.h>
#include <os/os.h>
#include <bsp/bsp.h>
#include <console/console.h>
#include <shell/shell.h>
#include <log/log.h>

#include <iotivity/oc_api.h>
#include "mn_socket/mn_socket.h"
#include "mn_socket/arch/sim/native_sock.h"

/* Shell */
#define SHELL_TASK_PRIO      (8)
#define SHELL_MAX_INPUT_LEN     (256)
#define SHELL_TASK_STACK_SIZE (OS_STACK_ALIGN(2048))
static os_stack_t shell_stack[SHELL_TASK_STACK_SIZE];

#define OCF_TASK_PRIO      (8)
#define OCF_TASK_STACK_SIZE (OS_STACK_ALIGN(2048))
static os_stack_t ocf_stack[OCF_TASK_STACK_SIZE];
struct os_task ocf_task;

static bool light_state = false;

#define DEFAULT_MBUF_MPOOL_BUF_LEN (256)
#define DEFAULT_MBUF_MPOOL_NBUFS (10)

static uint8_t default_mbuf_mpool_data[DEFAULT_MBUF_MPOOL_BUF_LEN *
    DEFAULT_MBUF_MPOOL_NBUFS];

static struct os_mbuf_pool default_mbuf_pool;
static struct os_mempool default_mbuf_mpool;

static void
get_light(oc_request_t *request, oc_interface_mask_t interface)
{
  PRINT("GET_light:\n");
  oc_rep_start_root_object();
  switch (interface) {
  case OC_IF_BASELINE:
    oc_process_baseline_interface(request->resource);
  case OC_IF_RW:
    oc_rep_set_boolean(root, state, light_state);
    break;
  default:
    break;
  }
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
  PRINT("Light state %d\n", light_state);
}

static void
put_light(oc_request_t *request, oc_interface_mask_t interface)
{
  PRINT("PUT_light:\n");
  bool state = false;
  oc_rep_t *rep = request->request_payload;
  while (rep != NULL) {
    PRINT("key: %s ", oc_string(rep->name));
    switch (rep->type) {
    case BOOL:
      state = rep->value_boolean;
      PRINT("value: %d\n", state);
      break;
    default:
      oc_send_response(request, OC_STATUS_BAD_REQUEST);
      return;
      break;
    }
    rep = rep->next;
  }
  oc_send_response(request, OC_STATUS_CHANGED);
  light_state = state;
}

static void
app_init(void)
{
  oc_init_platform("Mynewt", NULL, NULL);
  oc_add_device("/oic/d", "oic.d.light", "MynewtLED", "1.0", "1.0", NULL, NULL);
}

static void
register_resources(void)
{
  oc_resource_t *res = oc_new_resource("/light/1", 1, 0);
  oc_resource_bind_resource_type(res, "oic.r.light");
  oc_resource_bind_resource_interface(res, OC_IF_RW);
  oc_resource_set_default_interface(res, OC_IF_RW);

#ifdef OC_SECURITY
  oc_resource_make_secure(res);
#endif

  oc_resource_set_discoverable(res);
  oc_resource_set_periodic_observable(res, 1);
  oc_resource_set_request_handler(res, OC_GET, get_light);
  oc_resource_set_request_handler(res, OC_PUT, put_light);
  oc_add_resource(res);
}

struct os_sem ocf_main_loop_sem;


 oc_handler_t ocf_handler = {.init = app_init,
                          .register_resources = register_resources };

 void
oc_signal_main_loop(void) {
     os_sem_release(&ocf_main_loop_sem);
}

void ocf_task_handler(void *arg) {
    /* TODO */
    oc_main_init(&ocf_handler);

    os_sem_init(&ocf_main_loop_sem, 1);

    while (1) {
        uint32_t ticks;
        oc_clock_time_t next_event;
        next_event = oc_main_poll();
        ticks = oc_clock_time();
        if (next_event == 0) {
            ticks = OS_TIMEOUT_NEVER;
        } else {
            ticks = next_event - ticks;
        }
        os_sem_pend(&ocf_main_loop_sem, ticks);
    }
    oc_main_shutdown();
}

void
ocf_task_init(void) {

    int rc;
    rc = os_task_init(&ocf_task, "ocf", ocf_task_handler, NULL,
            OCF_TASK_PRIO, OS_WAIT_FOREVER, ocf_stack, OCF_TASK_STACK_SIZE);
    assert(rc == 0);
}

int
main(int argc, char **argv)
{
    int rc;

    /* Initialize OS */
    os_init();

    rc = os_mempool_init(&default_mbuf_mpool, DEFAULT_MBUF_MPOOL_NBUFS,
            DEFAULT_MBUF_MPOOL_BUF_LEN, default_mbuf_mpool_data,
            "default_mbuf_data");
    assert(rc == 0);

    rc = os_mbuf_pool_init(&default_mbuf_pool, &default_mbuf_mpool,
            DEFAULT_MBUF_MPOOL_BUF_LEN, DEFAULT_MBUF_MPOOL_NBUFS);
    assert(rc == 0);

    rc = os_msys_register(&default_mbuf_pool);
    assert(rc == 0);

    /* Init tasks */
    rc = shell_task_init(SHELL_TASK_PRIO, shell_stack, SHELL_TASK_STACK_SIZE,
                         SHELL_MAX_INPUT_LEN);
    assert(rc == 0);

    rc = native_sock_init();
    assert(rc == 0);

    ocf_task_init();

    /* Start the OS */
    os_start();

    /* os start should never return. If it does, this should be an error */
    assert(0);

}