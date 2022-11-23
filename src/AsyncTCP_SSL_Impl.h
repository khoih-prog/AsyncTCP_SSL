/****************************************************************************************************************************
  AsyncTCP_SSL_Impl.h

  AsyncTCP_SSL is a library for ESP32

  Based on and modified from :

  1) AsyncTCP (https://github.com/me-no-dev/ESPAsyncTCP)
  2) AsyncTCP (https://github.com/tve/AsyncTCP)

  Built by Khoi Hoang https://github.com/khoih-prog/AsyncTCP_SSL

  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
  as published bythe Free Software Foundation, either version 3 of the License, or (at your option) any later version.
  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  Version: 1.3.1

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0    K Hoang     21/10/2021 Initial coding to support only ESP32
  1.1.0    K Hoang     22/10/2021 Fix bug. Enable coexistence with AsyncTCP
  1.2.0    K Hoang     23/01/2022 Fix `multiple-definitions` linker error
  1.3.0    K Hoang     04/09/2022 Clean up. Remove hard-code if possible
  1.3.1    K Hoang     18/09/2022 Improve stability. Make queue length user-configurable
 *****************************************************************************************************************************/

/*
  Asynchronous TCP library for Espressif MCUs

  Copyright (c) 2016 Hristo Gochkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ASYNCTCP_SSL_IML_H
#define ASYNCTCP_SSL_IML_H

#include "Arduino.h"

#include "AsyncTCP_SSL_Debug.h"

extern "C"
{
#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/inet.h"
#include "lwip/dns.h"
#include "lwip/err.h"
}

#include "esp_task_wdt.h"

#define CONFIG_ASYNC_TCP_STACK      (2*8192)

#define ASYNC_TCP_SSL_DEBUG(...)

//////////////////////////////////////////////////////////////////////////////////////////

/*
   TCP/IP Event Task

   This task processes events that correspond to the various callbacks made by LwIP. The callbacks
   are handled by _tcp_* functions, which package the info into events, which are processed by this
   task. The purpose of this scheme is ??? (to be able to block or spend arbitrary time in the event
   handlers without thereby blocking LwIP???).
 * */

typedef enum
{
  LWIP_TCP_SENT,
  LWIP_TCP_RECV,
  LWIP_TCP_FIN,
  LWIP_TCP_ERROR,
  LWIP_TCP_POLL,
  LWIP_TCP_CLEAR,
  LWIP_TCP_ACCEPT,
  LWIP_TCP_CONNECTED,
  LWIP_TCP_DNS
} lwip_event_t;

typedef struct
{
  lwip_event_t event;
  void *arg;

  union
  {
    struct
    {
      void * pcb;
      int8_t err;
    } connected;

    struct
    {
      int8_t err;
    } error;

    struct
    {
      tcp_pcb * pcb;
      uint16_t len;
    } sent;

    struct
    {
      tcp_pcb * pcb;
      pbuf * pb;
      int8_t err;
    } recv;

    struct
    {
      tcp_pcb * pcb;
      int8_t err;
    } fin;

    struct
    {
      tcp_pcb * pcb;
    } poll;

    struct
    {
      AsyncSSLClient * client;
    } accept;

    struct
    {
      const char * name;
      ip_addr_t addr;
    } dns;
  };
} lwip_event_packet_t;

/////////////////////////////////////////////////

static xQueueHandle _async_queue;
static TaskHandle_t _async_service_task_handle = NULL;

static SemaphoreHandle_t _slots_lock;
const int _number_of_closed_slots = CONFIG_LWIP_MAX_ACTIVE_TCP;
static int _closed_slots[_number_of_closed_slots];

/////////////////////////////////////////////

static int _closed_index = []()
{
  _slots_lock = xSemaphoreCreateBinary();
  xSemaphoreGive(_slots_lock);

  for (int i = 0; i < _number_of_closed_slots; ++ i)
  {
    _closed_slots[i] = 1;
  }

  return 1;
}
();

/////////////////////////////////////////////

static inline bool _init_async_event_queue()
{
  if (!_async_queue)
  {
    _async_queue = xQueueCreate(ASYNC_QUEUE_LENGTH, sizeof(lwip_event_packet_t *));

    if (!_async_queue)
    {
      return false;
    }
  }

  return true;
}

/////////////////////////////////////////////

static inline bool _send_async_event(lwip_event_packet_t ** e)
{
  return _async_queue && xQueueSend(_async_queue, e, portMAX_DELAY) == pdPASS;
}

/////////////////////////////////////////////

static inline bool _prepend_async_event(lwip_event_packet_t ** e)
{
  return _async_queue && xQueueSendToFront(_async_queue, e, portMAX_DELAY) == pdPASS;
}

/////////////////////////////////////////////

static inline bool _get_async_event(lwip_event_packet_t ** e)
{
  return _async_queue && xQueueReceive(_async_queue, e, portMAX_DELAY) == pdPASS;
}

/////////////////////////////////////////////

static bool _remove_events_with_arg(void * arg)
{
  lwip_event_packet_t * first_packet = NULL;
  lwip_event_packet_t * packet       = NULL;

  if (!_async_queue)
  {
    return false;
  }

  //figure out which is the first packet so we can keep the order
  while (!first_packet)
  {
    if (xQueueReceive(_async_queue, &first_packet, 0) != pdPASS)
    {
      return false;
    }

    //discard packet if matching
    if ((int)first_packet->arg == (int)arg)
    {
      free(first_packet);
      first_packet = NULL;
      //return first packet to the back of the queue
    }
    else if (xQueueSend(_async_queue, &first_packet, portMAX_DELAY) != pdPASS)
    {
      return false;
    }
  }

  while (xQueuePeek(_async_queue, &packet, 0) == pdPASS && packet != first_packet)
  {
    if (xQueueReceive(_async_queue, &packet, 0) != pdPASS)
    {
      return false;
    }

    if ((int)packet->arg == (int)arg)
    {
      free(packet);
      packet = NULL;
    }
    else if (xQueueSend(_async_queue, &packet, portMAX_DELAY) != pdPASS)
    {
      return false;
    }
  }

  return true;
}

/////////////////////////////////////////////

static void _handle_async_event(lwip_event_packet_t * e)
{
  ATCP_LOGDEBUG1("_handle_async_event: Task Name = ", pcTaskGetTaskName(xTaskGetCurrentTaskHandle()));

  if (e->event == LWIP_TCP_CLEAR)
  {
    _remove_events_with_arg(e->arg);
  }
  else if (e->event == LWIP_TCP_RECV)
  {
    ATCP_HEXLOGINFO1("_handle_async_event: LWIP_TCP_RECV =", (uint32_t) e->recv.pcb);
    AsyncSSLClient::_s_recv(e->arg, e->recv.pcb, e->recv.pb, e->recv.err);
  }
  else if (e->event == LWIP_TCP_FIN)
  {
    ATCP_HEXLOGINFO1("_handle_async_event: LWIP_TCP_FIN =", (uint32_t) e->fin.pcb);
    AsyncSSLClient::_s_fin(e->arg, e->fin.pcb, e->fin.err);
  }
  else if (e->event == LWIP_TCP_SENT)
  {
    ATCP_HEXLOGINFO1("_handle_async_event: LWIP_TCP_SENT =", (uint32_t) e->sent.pcb);
    AsyncSSLClient::_s_sent(e->arg, e->sent.pcb, e->sent.len);
  }
  else if (e->event == LWIP_TCP_POLL)
  {
    ATCP_HEXLOGDEBUG1("_handle_async_event: LWIP_TCP_POLL =", (uint32_t) e->poll.pcb);
    AsyncSSLClient::_s_poll(e->arg, e->poll.pcb);
  }
  else if (e->event == LWIP_TCP_ERROR)
  {
    ATCP_HEXLOGINFO1("_handle_async_event: LWIP_TCP_ERROR =", (uint32_t) e->arg);
    ATCP_LOGINFO1("_handle_async_event: LWIP_TCP_ERROR = ", e->error.err);
    AsyncSSLClient::_s_error(e->arg, e->error.err);
  }
  else if (e->event == LWIP_TCP_CONNECTED)
  {
    ATCP_HEXLOGINFO2("_handle_async_event: LWIP_TCP_CONNECTED =", (uint32_t) e->arg, (uint32_t) e->connected.pcb);
    ATCP_LOGINFO1("_handle_async_event: LWIP_TCP_CONNECTED = ", e->connected.err);
    AsyncSSLClient::_s_connected(e->arg, e->connected.pcb, e->connected.err);
  }
  else if (e->event == LWIP_TCP_ACCEPT)
  {
    ATCP_HEXLOGINFO2("_handle_async_event: LWIP_TCP_ACCEPT =", (uint32_t) e->arg, (uint32_t) e->accept.client);
    AsyncSSLServer::_s_accepted(e->arg, e->accept.client);
  }
  else if (e->event == LWIP_TCP_DNS)
  {
    ATCP_HEXLOGINFO1("_handle_async_event: LWIP_TCP_DNS =", (uint32_t) e->arg);
    ATCP_LOGINFO3("_handle_async_event: LWIP_TCP_DNS, name =", e->dns.name, ", IP =", ipaddr_ntoa(&e->dns.addr));
    AsyncSSLClient::_s_dns_found(e->dns.name, &e->dns.addr, e->arg);
  }

  free((void*)(e));
}

/////////////////////////////////////////////

static void _async_service_task(void *pvParameters)
{
  lwip_event_packet_t * packet = NULL;

  for (;;)
  {
    if (_get_async_event(&packet))
    {
#if CONFIG_ASYNC_TCP_USE_WDT

      if (esp_task_wdt_add(NULL) != ESP_OK)
      {
        ATCP_LOGERROR("Failed to add async task to WDT");
      }

#endif

      if (packet)
        _handle_async_event(packet);
      else
      {
        ATCP_LOGERROR("_async_service_task, NUL packet");
      }

#if CONFIG_ASYNC_TCP_USE_WDT

      if (esp_task_wdt_delete(NULL) != ESP_OK)
      {
        ATCP_LOGERROR("Failed to remove loop task from WDT");
      }

#endif
    }
  }

  vTaskDelete(NULL);
  _async_service_task_handle = NULL;
}

/////////////////////////////////////////////

/*
  static void _stop_async_task(){
    if(_async_service_task_handle){
        vTaskDelete(_async_service_task_handle);
        _async_service_task_handle = NULL;
    }
  }
*/

/////////////////////////////////////////////

static bool _start_async_task()
{
  if (!_init_async_event_queue())
  {
    return false;
  }

  if (!_async_service_task_handle)
  {
    xTaskCreateUniversal(_async_service_task, "async_tcp_ssl", CONFIG_ASYNC_TCP_STACK, NULL, CONFIG_ASYNC_TCP_PRIORITY,
                         &_async_service_task_handle, CONFIG_ASYNC_TCP_RUNNING_CORE);

    if (!_async_service_task_handle)
    {
      return false;
    }
  }

  return true;
}

/////////////////////////////////////////////

/*
   LwIP Callbacks

   The following "_tcp_*" functions are called by LwIP on its thread. They all do nothing but
   package the callback info into an event, which is queued for the async event task (see above).
 * */

static int8_t _tcp_clear_events(void * arg)
{
  lwip_event_packet_t * e = (lwip_event_packet_t *) malloc(sizeof(lwip_event_packet_t));
  e->event = LWIP_TCP_CLEAR;
  e->arg = arg;

  if (!_prepend_async_event(&e))
  {
    free((void*)(e));
  }

  return ERR_OK;
}

/////////////////////////////////////////////

static int8_t _tcp_connected(void * arg, tcp_pcb * pcb, int8_t err)
{
  ATCP_HEXLOGDEBUG1("_tcp_connected: pcb =", (uint32_t) pcb);

  lwip_event_packet_t * e = (lwip_event_packet_t *) malloc(sizeof(lwip_event_packet_t));
  e->event = LWIP_TCP_CONNECTED;
  e->arg = arg;
  e->connected.pcb = pcb;
  e->connected.err = err;

  if (!_prepend_async_event(&e))
  {
    free((void*)(e));
  }

  return ERR_OK;
}

/////////////////////////////////////////////

static int8_t _tcp_poll(void * arg, struct tcp_pcb * pcb)
{
  ATCP_HEXLOGDEBUG1("_tcp_poll: pcb =", (uint32_t) pcb);

  lwip_event_packet_t * e = (lwip_event_packet_t *) malloc(sizeof(lwip_event_packet_t));
  e->event = LWIP_TCP_POLL;
  e->arg = arg;
  e->poll.pcb = pcb;

  if (!_send_async_event(&e))
  {
    free((void*)(e));
  }

  return ERR_OK;
}

/////////////////////////////////////////////

static int8_t _tcp_recv(void * arg, struct tcp_pcb * pcb, struct pbuf *pb, int8_t err)
{
  lwip_event_packet_t * e = (lwip_event_packet_t *) malloc(sizeof(lwip_event_packet_t));
  e->arg = arg;

  if (pb)
  {
    ATCP_HEXLOGDEBUG1("_tcp_recv: pcb =", (uint32_t) pcb);

    e->event = LWIP_TCP_RECV;
    e->recv.pcb = pcb;
    e->recv.pb = pb;
    e->recv.err = err;
  }
  else
  {
    ATCP_HEXLOGDEBUG1("_tcp_recv: failed, pcb =", (uint32_t) pcb);

    e->event = LWIP_TCP_FIN;
    e->fin.pcb = pcb;
    e->fin.err = err;
    //close the PCB in LwIP thread
    AsyncSSLClient::_s_lwip_fin(e->arg, e->fin.pcb, e->fin.err);
  }

  if (!_send_async_event(&e))
  {
    free((void*)(e));
  }

  return ERR_OK;
}

/////////////////////////////////////////////

static int8_t _tcp_sent(void * arg, struct tcp_pcb * pcb, uint16_t len)
{
  ATCP_HEXLOGDEBUG1("_tcp_sent: pcb =", (uint32_t) pcb);

  lwip_event_packet_t * e = (lwip_event_packet_t *) malloc(sizeof(lwip_event_packet_t));
  e->event = LWIP_TCP_SENT;
  e->arg = arg;
  e->sent.pcb = pcb;
  e->sent.len = len;

  if (!_send_async_event(&e))
  {
    free((void*)(e));
  }

  return ERR_OK;
}

/////////////////////////////////////////////

static void _tcp_error(void * arg, int8_t err)
{
  ATCP_HEXLOGDEBUG1("_tcp_error: arg =", (uint32_t) arg);

  lwip_event_packet_t * e = (lwip_event_packet_t *) malloc(sizeof(lwip_event_packet_t));
  e->event = LWIP_TCP_ERROR;
  e->arg = arg;
  e->error.err = err;

  if (!_send_async_event(&e))
  {
    free((void*)(e));
  }
}

/////////////////////////////////////////////

static void _tcp_dns_found(const char * name, struct ip_addr * ipaddr, void * arg)
{
  lwip_event_packet_t * e = (lwip_event_packet_t *) malloc(sizeof(lwip_event_packet_t));

  ATCP_LOGDEBUG3("_tcp_dns_found: name =", name, ", IP =", ipaddr_ntoa(ipaddr));
  ATCP_HEXLOGDEBUG1("_tcp_dns_found: arg =", (uint32_t) arg);

  e->event = LWIP_TCP_DNS;
  e->arg = arg;
  e->dns.name = name;

  if (ipaddr)
  {
    memcpy(&e->dns.addr, ipaddr, sizeof(struct ip_addr));
  }
  else
  {
    memset(&e->dns.addr, 0, sizeof(e->dns.addr));
  }

  if (!_send_async_event(&e))
  {
    free((void*)(e));
  }
}

/////////////////////////////////////////////

//Used to switch out from LwIP thread
static int8_t _tcp_accept(void * arg, AsyncSSLClient * client)
{
  lwip_event_packet_t * e = (lwip_event_packet_t *) malloc(sizeof(lwip_event_packet_t));
  e->event = LWIP_TCP_ACCEPT;
  e->arg = arg;
  e->accept.client = client;

  if (!_prepend_async_event(&e))
  {
    free((void*)(e));

    // KH Test Memory Leak
    if (client)
    {
      ATCP_LOGDEBUG("AsyncTCP_SSL: _tcp_accept: Delete Client");
      delete (client);
      client = nullptr;
    }

    //////
  }

  return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
   TCP/IP API Calls

   The following functions provide stubs to call into LwIP's TCP api functions on the LwIP thread
   itself. This ensures there are no race conditions between the application and LwIP.
   The way it works is that the `_tcp_xxx` functions synchronously call the corresponding
   `_tcp_xxx_api` functions on the LwIP thread using a `tcp_api_call` mechanism provided by LwIP.
   The `_tcp_xxx_api` function then finally calls the actual `tcp_xxx` function in LwIP and returns
   the result.
 * */

#include "lwip/priv/tcpip_priv.h"

typedef struct
{
  struct tcpip_api_call_data call;
  tcp_pcb * pcb;
  int8_t closed_slot;
  int8_t err;

  union
  {
    struct
    {
      const char* data;
      size_t size;
      uint8_t apiflags;
    } write;

    size_t received;

    struct
    {
      ip_addr_t * addr;
      uint16_t port;
      tcp_connected_fn cb;
    } connect;

    struct
    {
      ip_addr_t * addr;
      uint16_t port;
    } bind;

    uint8_t backlog;
  };
} tcp_api_call_t;

/////////////////////////////////////////////

static err_t _tcp_output_api(struct tcpip_api_call_data *api_call_msg)
{
  tcp_api_call_t * msg = (tcp_api_call_t *)api_call_msg;

  msg->err = ERR_CONN;

  if (msg->closed_slot == INVALID_CLOSED_SLOT || !_closed_slots[msg->closed_slot])
  {
    msg->err = tcp_output(msg->pcb);
  }

  return msg->err;
}

/////////////////////////////////////////////

static esp_err_t _tcp_output(tcp_pcb * pcb, int8_t closed_slot)
{
  if (!pcb)
  {
    return ERR_CONN;
  }

  tcp_api_call_t msg;

  msg.pcb         = pcb;
  msg.closed_slot = closed_slot;

  tcpip_api_call(_tcp_output_api, (struct tcpip_api_call_data*)&msg);

  return msg.err;
}

/////////////////////////////////////////////

static err_t _tcp_write_api(struct tcpip_api_call_data *api_call_msg)
{
  tcp_api_call_t * msg = (tcp_api_call_t *)api_call_msg;

  msg->err = ERR_CONN;

  if (msg->closed_slot == INVALID_CLOSED_SLOT || !_closed_slots[msg->closed_slot])
  {
    msg->err = tcp_write(msg->pcb, msg->write.data, msg->write.size, msg->write.apiflags);
  }

  return msg->err;
}

/////////////////////////////////////////////

static esp_err_t _tcp_write(tcp_pcb * pcb, int8_t closed_slot, const char* data, size_t size, uint8_t apiflags)
{
  if (!pcb)
  {
    return ERR_CONN;
  }

  tcp_api_call_t msg;

  msg.pcb            = pcb;
  msg.closed_slot    = closed_slot;
  msg.write.data     = data;
  msg.write.size     = size;
  msg.write.apiflags = apiflags;

  tcpip_api_call(_tcp_write_api, (struct tcpip_api_call_data*)&msg);

  return msg.err;
}

/////////////////////////////////////////////

static err_t _tcp_recved_api(struct tcpip_api_call_data *api_call_msg)
{
  tcp_api_call_t * msg = (tcp_api_call_t *)api_call_msg;

  msg->err = ERR_CONN;

  if (msg->closed_slot == INVALID_CLOSED_SLOT || !_closed_slots[msg->closed_slot])
  {
    msg->err = 0;

    tcp_recved(msg->pcb, msg->received);
  }

  return msg->err;
}

/////////////////////////////////////////////

static esp_err_t _tcp_recved(tcp_pcb * pcb, int8_t closed_slot, size_t len)
{
  if (!pcb)
  {
    return ERR_CONN;
  }

  tcp_api_call_t msg;

  msg.pcb         = pcb;
  msg.closed_slot = closed_slot;
  msg.received    = len;

  tcpip_api_call(_tcp_recved_api, (struct tcpip_api_call_data*)&msg);

  return msg.err;
}

/////////////////////////////////////////////

static err_t _tcp_close_api(struct tcpip_api_call_data *api_call_msg)
{
  tcp_api_call_t * msg = (tcp_api_call_t *)api_call_msg;

  msg->err = ERR_CONN;

  if (msg->closed_slot == INVALID_CLOSED_SLOT || !_closed_slots[msg->closed_slot])
  {
    msg->err = tcp_close(msg->pcb);
  }

  return msg->err;
}

/////////////////////////////////////////////

static esp_err_t _tcp_close(tcp_pcb * pcb, int8_t closed_slot)
{
  if (!pcb)
  {
    return ERR_CONN;
  }

  tcp_api_call_t msg;

  msg.pcb         = pcb;
  msg.closed_slot = closed_slot;

  tcpip_api_call(_tcp_close_api, (struct tcpip_api_call_data*)&msg);

  return msg.err;
}

/////////////////////////////////////////////

static err_t _tcp_abort_api(struct tcpip_api_call_data *api_call_msg)
{
  tcp_api_call_t * msg = (tcp_api_call_t *)api_call_msg;

  msg->err = ERR_CONN;

  if (msg->closed_slot == INVALID_CLOSED_SLOT || !_closed_slots[msg->closed_slot])
  {
    tcp_abort(msg->pcb);
  }

  return msg->err;
}

/////////////////////////////////////////////

static esp_err_t _tcp_abort(tcp_pcb * pcb, int8_t closed_slot)
{
  if (!pcb)
  {
    return ERR_CONN;
  }

  tcp_api_call_t msg;

  msg.pcb         = pcb;
  msg.closed_slot = closed_slot;

  tcpip_api_call(_tcp_abort_api, (struct tcpip_api_call_data*)&msg);

  return msg.err;
}

/////////////////////////////////////////////

static err_t _tcp_connect_api(struct tcpip_api_call_data *api_call_msg)
{
  tcp_api_call_t * msg = (tcp_api_call_t *)api_call_msg;

  msg->err = tcp_connect(msg->pcb, msg->connect.addr, msg->connect.port, msg->connect.cb);

  return msg->err;
}

/////////////////////////////////////////////

static esp_err_t _tcp_connect(tcp_pcb * pcb, int8_t closed_slot, ip_addr_t * addr, uint16_t port, tcp_connected_fn cb)
{
  if (!pcb)
  {
    return ESP_FAIL;
  }

  tcp_api_call_t msg;

  msg.pcb          = pcb;
  msg.closed_slot  = closed_slot;
  msg.connect.addr = addr;
  msg.connect.port = port;
  msg.connect.cb   = cb;

  tcpip_api_call(_tcp_connect_api, (struct tcpip_api_call_data*)&msg);

  return msg.err;
}

/////////////////////////////////////////////

static err_t _tcp_bind_api(struct tcpip_api_call_data *api_call_msg)
{
  tcp_api_call_t * msg = (tcp_api_call_t *)api_call_msg;

  msg->err = tcp_bind(msg->pcb, msg->bind.addr, msg->bind.port);

  return msg->err;
}

/////////////////////////////////////////////

static esp_err_t _tcp_bind(tcp_pcb * pcb, ip_addr_t * addr, uint16_t port)
{
  if (!pcb)
  {
    return ESP_FAIL;
  }

  tcp_api_call_t msg;

  msg.pcb         = pcb;
  msg.closed_slot = INVALID_CLOSED_SLOT;
  msg.bind.addr   = addr;
  msg.bind.port   = port;

  tcpip_api_call(_tcp_bind_api, (struct tcpip_api_call_data*)&msg);

  return msg.err;
}

/////////////////////////////////////////////

static err_t _tcp_listen_api(struct tcpip_api_call_data *api_call_msg)
{
  tcp_api_call_t * msg = (tcp_api_call_t *)api_call_msg;

  msg->err = 0;

  msg->pcb = tcp_listen_with_backlog(msg->pcb, msg->backlog);

  return msg->err;
}

/////////////////////////////////////////////

static tcp_pcb * _tcp_listen_with_backlog(tcp_pcb * pcb, uint8_t backlog)
{
  if (!pcb)
  {
    return NULL;
  }

  tcp_api_call_t msg;

  msg.pcb         = pcb;
  msg.closed_slot = INVALID_CLOSED_SLOT;
  msg.backlog     = backlog ? backlog : 0xFF;

  tcpip_api_call(_tcp_listen_api, (struct tcpip_api_call_data*)&msg);

  return msg.pcb;
}

/////////////////////////////////////////////

// KH
extern "C"
{
  // The following API stubs are for use in tcp_mbedtls.c
  // They are callable from C and take a void* instead of an AsyncSSLClient*.

  esp_err_t _tcp_output4ssl(tcp_pcb * pcb, void* client)
  {
    // KH
    return _tcp_output(pcb, (reinterpret_cast<AsyncSSLClient *> (client) )->getClosed_Slot() );
    //////
  }

  esp_err_t _tcp_write4ssl(tcp_pcb * pcb, const char* data, size_t size, uint8_t apiflags, void* client)
  {
    // KH
    return _tcp_write(pcb, (reinterpret_cast<AsyncSSLClient *> (client) )->getClosed_Slot(), data, size, apiflags);
    //////
  }

}

//////////////////////////////////////////////////////////////////////////////////////

/*
  Async TCP Client
*/

AsyncSSLClient::AsyncSSLClient(tcp_pcb* pcb)
  : _connect_cb(0)
  , _connect_cb_arg(0)
  , _discard_cb(0)
  , _discard_cb_arg(0)
  , _sent_cb(0)
  , _sent_cb_arg(0)
  , _error_cb(0)
  , _error_cb_arg(0)
  , _recv_cb(0)
  , _recv_cb_arg(0)
  , _pb_cb(0)
  , _pb_cb_arg(0)
  , _timeout_cb(0)
  , _timeout_cb_arg(0)
  , _pcb_busy(false)
  , _pcb_sent_at(0)
  , _ack_pcb(true)
  , _rx_last_packet(0)
  , _rx_since_timeout(0)
  , _ack_timeout(ASYNC_MAX_ACK_TIME)
  , _connect_port(0)
    // SSL
  , _root_ca_len(0)
  , _root_ca(NULL)
  , _cli_cert_len(0)
  , _cli_cert(NULL)
  , _cli_key_len(0)
  , _cli_key(NULL)
  , _pcb_secure(false)
  , _handshake_done(true)
  , _psk_ident(0)
  , _psk(0)
    //////
  , prev(NULL)
  , next(NULL)
{
  _pcb = pcb;
  _closed_slot = INVALID_CLOSED_SLOT;

  if (_pcb)
  {
    _allocate_closed_slot();
    _rx_last_packet = millis();
    tcp_arg(_pcb, this);
    tcp_recv(_pcb, &_tcp_recv);
    tcp_sent(_pcb, &_tcp_sent);
    tcp_err(_pcb, &_tcp_error);
    tcp_poll(_pcb, &_tcp_poll, 1);
  }
}

/////////////////////////////////////////////

AsyncSSLClient::~AsyncSSLClient()
{
  if (_pcb)
  {
    _close();
  }

  _free_closed_slot();
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
   Operators
 * */

AsyncSSLClient& AsyncSSLClient::operator=(const AsyncSSLClient& other)
{
  if (_pcb)
  {
    _close();
  }

  _pcb = other._pcb;
  _closed_slot = other._closed_slot;

  if (_pcb)
  {
    _rx_last_packet = millis();
    tcp_arg(_pcb, this);
    tcp_recv(_pcb, &_tcp_recv);
    tcp_sent(_pcb, &_tcp_sent);
    tcp_err(_pcb, &_tcp_error);
    tcp_poll(_pcb, &_tcp_poll, 1);

    // SSL
    if (tcp_ssl_has(_pcb))
    {
      _pcb_secure     = true;
      _handshake_done = false;

      tcp_ssl_arg(_pcb, this);
      tcp_ssl_data(_pcb, &_s_data);
      tcp_ssl_handshake(_pcb, &_s_handshake);
      tcp_ssl_err(_pcb, &_s_ssl_error);
    }
    else
    {
      _pcb_secure = false;
      _handshake_done = true;
    }

    //////
  }

  return *this;
}

/////////////////////////////////////////////

bool AsyncSSLClient::operator==(const AsyncSSLClient &other)
{
  return (_pcb == other._pcb);
}

/////////////////////////////////////////////

AsyncSSLClient & AsyncSSLClient::operator+=(const AsyncSSLClient &other)
{
  if (next == NULL)
  {
    next = (AsyncSSLClient*)(&other);
    next->prev = this;
  }
  else
  {
    AsyncSSLClient *c = next;

    while (c->next != NULL)
    {
      c = c->next;
    }

    c->next = (AsyncSSLClient*)(&other);
    c->next->prev = c;
  }

  return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
   Callback Setters
 * */

void AsyncSSLClient::onConnect(AcConnectHandlerSSL cb, void* arg)
{
  _connect_cb     = cb;
  _connect_cb_arg = arg;
}

/////////////////////////////////////////////

void AsyncSSLClient::onDisconnect(AcConnectHandlerSSL cb, void* arg)
{
  _discard_cb     = cb;
  _discard_cb_arg = arg;
}

/////////////////////////////////////////////

void AsyncSSLClient::onAck(AcAckHandlerSSL cb, void* arg)
{
  _sent_cb     = cb;
  _sent_cb_arg = arg;
}

/////////////////////////////////////////////

void AsyncSSLClient::onError(AcErrorHandlerSSL cb, void* arg)
{
  _error_cb     = cb;
  _error_cb_arg = arg;
}

/////////////////////////////////////////////

void AsyncSSLClient::onData(AcDataHandlerSSL cb, void* arg)
{
  _recv_cb     = cb;
  _recv_cb_arg = arg;
}

/////////////////////////////////////////////

void AsyncSSLClient::onPacket(AcPacketHandlerSSL cb, void* arg)
{
  _pb_cb     = cb;
  _pb_cb_arg = arg;
}

/////////////////////////////////////////////

void AsyncSSLClient::onTimeout(AcTimeoutHandlerSSL cb, void* arg)
{
  _timeout_cb     = cb;
  _timeout_cb_arg = arg;
}

/////////////////////////////////////////////

void AsyncSSLClient::onPoll(AcConnectHandlerSSL cb, void* arg)
{
  _poll_cb     = cb;
  _poll_cb_arg = arg;
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
   Main Public Methods
 * */

bool AsyncSSLClient::connect(IPAddress ip, uint16_t port, bool secure)
{
  if (_pcb)
  {
    ATCP_LOGWARN1("connect: already connected, state =", stateToString());

    return false;
  }

  if (!_start_async_task())
  {
    ATCP_LOGERROR("connect: failed to start task");

    return false;
  }

  ip_addr_t addr;
  addr.type = IPADDR_TYPE_V4;
  addr.u_addr.ip4.addr = ip;

  tcp_pcb* pcb = tcp_new_ip_type(IPADDR_TYPE_V4);

  if (!pcb)
  {
    ATCP_LOGERROR("connect: NULL pcb");

    return false;
  }

  // SSL
  _pcb_secure = secure;
  _handshake_done = !secure;
  //////

  tcp_arg(pcb, this);
  tcp_err(pcb, &_tcp_error);
  tcp_recv(pcb, &_tcp_recv);
  tcp_sent(pcb, &_tcp_sent);
  tcp_poll(pcb, &_tcp_poll, 1);

  _tcp_connect(pcb, _closed_slot, &addr, port, (tcp_connected_fn)&_tcp_connected);

  return true;
}

/////////////////////////////////////////////

bool AsyncSSLClient::connect(const char* host, uint16_t port, bool secure)
{
  ip_addr_t addr;

  if (!_start_async_task())
  {
    ATCP_LOGERROR("connect: failed to start task");

    return false;
  }

  err_t err = dns_gethostbyname(host, &addr, (dns_found_callback)&_tcp_dns_found, this);

  if (err == ERR_OK)
  {
    _hostname = host;

    return connect(IPAddress(addr.u_addr.ip4.addr), port, secure);
  }
  else if (err == ERR_INPROGRESS)
  {
    _connect_port = port;

    _hostname = host;
    _pcb_secure = secure;
    _handshake_done = !secure;

    return true;
  }

  ATCP_LOGERROR1("connect: error =", err);

  return false;
}

/////////////////////////////////////////////

void AsyncSSLClient::close(bool now)
{
  if (_pcb)
  {
    _tcp_recved(_pcb, _closed_slot, _rx_ack_len);
  }

  _close();
}

/////////////////////////////////////////////

int8_t AsyncSSLClient::abort()
{
  if (_pcb)
  {
    _tcp_abort(_pcb, _closed_slot );
    _pcb = NULL;
  }

  return ERR_ABRT;
}

/////////////////////////////////////////////

void AsyncSSLClient::setRootCa(const char* rootca, const size_t len)
{
  _root_ca     = (char*)rootca;
  _root_ca_len = len;
}

/////////////////////////////////////////////

void AsyncSSLClient::setClientCert(const char* cli_cert, const size_t len)
{
  _cli_cert     = (char*)cli_cert;
  _cli_cert_len = len;
}

/////////////////////////////////////////////

void AsyncSSLClient::setClientKey(const char* cli_key, const size_t len)
{
  _cli_key     = (char*)cli_key;
  _cli_key_len = len;
}

/////////////////////////////////////////////

void AsyncSSLClient::setPsk(const char* psk_ident, const char* psk)
{
  _psk_ident = psk_ident;
  _psk       = psk;
}


/////////////////////////////////////////////

size_t AsyncSSLClient::space()
{
  if ((_pcb != NULL) && (_pcb->state == ESTABLISHED))
  {
    return tcp_sndbuf(_pcb);
  }

  return 0;
}

/////////////////////////////////////////////

size_t AsyncSSLClient::add(const char* data, size_t size, uint8_t apiflags)
{
  if (!_pcb || size == 0 || data == NULL)
  {
    return 0;
  }

  size_t room = space();

  if (!room)
  {
    return 0;
  }

  if (_pcb_secure)
  {
    int sent = tcp_ssl_write(_pcb, (uint8_t*)data, size);

    ASYNC_TCP_SSL_DEBUG("add() tcp_ssl_write size = %d\n", sent);

    if (sent >= 0)
    {
      // @ToDo: ???
      //_tx_unacked_len += sent;
      return sent;
    }

    ATCP_LOGINFO1("add() done, tcp_ssl_write size =", sent);

    _close();

    return 0;
  }

  size_t will_send = (room < size) ? room : size;

  int8_t err = ERR_OK;

  err = _tcp_write(_pcb, _closed_slot, data, will_send, apiflags);

  if (err != ERR_OK)
  {
    return 0;
  }

  return will_send;
}

/////////////////////////////////////////////

bool AsyncSSLClient::send()
{
  // 5 is also OK
  vTaskDelay(1 / portTICK_PERIOD_MS);

  int8_t err = ERR_OK;

  err = _tcp_output(_pcb, _closed_slot);

  if (err == ERR_OK)
  {
    _pcb_busy    = true;
    _pcb_sent_at = millis();

    return true;
  }

  return false;
}

/////////////////////////////////////////////

size_t AsyncSSLClient::ack(size_t len)
{
  if (len > _rx_ack_len)
    len = _rx_ack_len;

  if (len)
  {
    _tcp_recved(_pcb, _closed_slot, len);
  }

  _rx_ack_len -= len;

  return len;
}

/////////////////////////////////////////////

void AsyncSSLClient::ackPacket(struct pbuf * pb)
{
  if (!pb)
  {
    return;
  }

  _tcp_recved(_pcb, _closed_slot, pb->len);
  pbuf_free(pb);
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
   Main Private Methods
 * */

int8_t AsyncSSLClient::_close()
{
  int8_t err = ERR_OK;

  if (_pcb)
  {
    if (_pcb_secure)
    {
      tcp_ssl_free(_pcb);
    }

    tcp_arg(_pcb, NULL);
    tcp_sent(_pcb, NULL);
    tcp_recv(_pcb, NULL);
    tcp_err(_pcb, NULL);
    tcp_poll(_pcb, NULL, 0);

    _tcp_clear_events(this);

    err = _tcp_close(_pcb, _closed_slot);

    if (err != ERR_OK)
    {
      err = abort();
    }

    _pcb = NULL;

    if (_discard_cb)
    {
      _discard_cb(_discard_cb_arg, this);
    }
  }

  return err;
}

/////////////////////////////////////////////

void AsyncSSLClient::_allocate_closed_slot()
{
  xSemaphoreTake(_slots_lock, portMAX_DELAY);

  uint32_t closed_slot_min_index = 0;

  for (int i = 0; i < _number_of_closed_slots; ++ i)
  {
    if ((_closed_slot == INVALID_CLOSED_SLOT || _closed_slots[i] <= closed_slot_min_index) && _closed_slots[i] != 0)
    {
      closed_slot_min_index = _closed_slots[i];
      _closed_slot = i;
    }
  }

  if (_closed_slot != INVALID_CLOSED_SLOT)
  {
    _closed_slots[_closed_slot] = 0;
  }

  xSemaphoreGive(_slots_lock);
}

/////////////////////////////////////////////

void AsyncSSLClient::_free_closed_slot()
{
  if (_closed_slot != INVALID_CLOSED_SLOT)
  {
    _closed_slots[_closed_slot] = _closed_index;
    _closed_slot = INVALID_CLOSED_SLOT;
    _closed_index++;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
   Private Callbacks
 * */

int8_t AsyncSSLClient::_connected(void* pcb, int8_t err)
{
  _pcb = reinterpret_cast<tcp_pcb*>(pcb);

  if (_pcb)
  {
    _rx_last_packet = millis();
    _pcb_busy = false;

    if (_pcb_secure)
    {
      bool err = false;

      if (_psk_ident != NULL and _psk != NULL)
      {
        err = tcp_ssl_new_psk_client(_pcb, this, _psk_ident, _psk) < 0;
      }
      else
      {
        err = tcp_ssl_new_client(_pcb, this, _hostname.empty() ? NULL : _hostname.c_str(),
                                 _root_ca, _root_ca_len, _cli_cert, _cli_cert_len, _cli_key, _cli_key_len) < 0;
      }

      if (err)
      {
        ATCP_LOGERROR("_connected: error => closing");

        return _close();
      }

      tcp_ssl_data(_pcb, &_s_data);
      tcp_ssl_handshake(_pcb, &_s_handshake);
      tcp_ssl_err(_pcb, &_s_ssl_error);
    }
  }

  // _connect_cb happens after SSL handshake if this is a secure connection
  if (_connect_cb && !_pcb_secure)
  {
    _connect_cb(_connect_cb_arg, this);
  }

  return ERR_OK;
}

/////////////////////////////////////////////

void AsyncSSLClient::_error(int8_t err)
{
  if (_pcb)
  {
    if (_pcb_secure)
    {
      tcp_ssl_free(_pcb);
    }

    tcp_arg(_pcb, NULL);

    if (_pcb->state == LISTEN)
    {
      tcp_sent(_pcb, NULL);
      tcp_recv(_pcb, NULL);
      tcp_err(_pcb, NULL);
      tcp_poll(_pcb, NULL, 0);
    }

    _pcb = NULL;
  }

  if (_error_cb)
  {
    _error_cb(_error_cb_arg, this, err);
  }

  if (_discard_cb)
  {
    _discard_cb(_discard_cb_arg, this);
  }
}

/////////////////////////////////////////////

void AsyncSSLClient::_ssl_error(int8_t err)
{
  if (_error_cb)
  {
    _error_cb(_error_cb_arg, this, err + 64);
  }
}

/////////////////////////////////////////////

//In LwIP Thread
int8_t AsyncSSLClient::_lwip_fin(tcp_pcb* pcb, int8_t err)
{
  if (!_pcb || pcb != _pcb)
  {
    ATCP_HEXLOGDEBUG2("_lwip_fin: pcb/_pcb =", (uint32_t)pcb, (uint32_t)_pcb);

    return ERR_OK;
  }

  tcp_arg(_pcb, NULL);

  if (_pcb->state == LISTEN)
  {
    tcp_sent(_pcb, NULL);
    tcp_recv(_pcb, NULL);
    tcp_err(_pcb, NULL);
    tcp_poll(_pcb, NULL, 0);
  }

  if (tcp_close(_pcb) != ERR_OK)
  {
    tcp_abort(_pcb);
  }

  _free_closed_slot();
  _pcb = NULL;

  return ERR_OK;
}

/////////////////////////////////////////////

//In Async Thread
int8_t AsyncSSLClient::_fin(tcp_pcb* pcb, int8_t err)
{
  _tcp_clear_events(this);

  if (_discard_cb)
  {
    _discard_cb(_discard_cb_arg, this);
  }

  return ERR_OK;
}

/////////////////////////////////////////////

int8_t AsyncSSLClient::_sent(tcp_pcb* pcb, uint16_t len)
{
  _rx_last_packet = millis();

  ATCP_LOGINFO1("_sent: len =", len);

  _pcb_busy = false;

  if (_sent_cb)
  {
    _sent_cb(_sent_cb_arg, this, len, (millis() - _pcb_sent_at));
  }

  return ERR_OK;
}

/////////////////////////////////////////////

int8_t AsyncSSLClient::_recv(tcp_pcb* pcb, pbuf* pb, int8_t err)
{
  while (pb != NULL)
  {
    _rx_last_packet = millis();

    pbuf *nxt = pb->next;
    pb->next  = NULL;

    if (_pcb_secure)
    {
      ATCP_LOGINFO1("_recv: tot_len =", pb->tot_len);

      int err = tcp_ssl_read(pcb, pb);
      // tcp_ssl_read always processes the full pbuf, so ack all of it

      // KH
      //_tcp_recved(pcb, pb->len);
      _tcp_recved(pcb, _closed_slot, pb->len);
      //////

      pbuf_free(pb);

      // handle errors
      if (err < 0)
      {
        if (err != MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
        {
          ATCP_LOGERROR1("_recv: err =", err);

          _close();
        }

        return ERR_BUF; // for lack of a better error value
      }

    }
    else
    {
      //we should not ack before we assimilate the data
      _ack_pcb = true;

      if (_pb_cb)
      {
        _pb_cb(_pb_cb_arg, this, pb);
      }
      else
      {
        if (_recv_cb)
        {
          _recv_cb(_recv_cb_arg, this, pb->payload, pb->len);
        }

        if (!_ack_pcb)
        {
          _rx_ack_len += pb->len;
        }
        else if (_pcb)
        {
          _tcp_recved(_pcb, _closed_slot, pb->len);
        }

        pbuf_free(pb);
      }
    }

    pb = nxt;
  }

  return ERR_OK;
}

/////////////////////////////////////////////

int8_t AsyncSSLClient::_poll(tcp_pcb* pcb)
{
  if (!_pcb)
  {
    ATCP_LOGWARN("_poll: NULL pcb");

    return ERR_OK;
  }

  if (pcb != _pcb)
  {
    ATCP_HEXLOGERROR2("_poll: diff pcb/_pcb =", (uint32_t) pcb, (uint32_t) _pcb);

    return ERR_OK;
  }

  uint32_t now = millis();

  // ACK Timeout
  if (_pcb_busy && _ack_timeout && (now - _pcb_sent_at) >= _ack_timeout)
  {
    _pcb_busy = false;

    ATCP_LOGWARN1("_poll: ack timeout, state =", stateToString());

    if (_timeout_cb)
      _timeout_cb(_timeout_cb_arg, this, (now - _pcb_sent_at));

    return ERR_OK;
  }

  // RX Timeout
  if (_rx_since_timeout && (now - _rx_last_packet) >= (_rx_since_timeout * 1000))
  {
    ATCP_LOGWARN1("_poll: rx timeout, state =", stateToString());

    _close();
    return ERR_OK;
  }

  if (_pcb_secure && !_handshake_done && (now - _rx_last_packet) >= SSL_HANDSHAKE_TIMEOUT)
  {
    ATCP_LOGWARN1("_poll: ssl handshake timeout, state =", stateToString());

    _close();

    return ERR_OK;
  }

  // Everything is fine
  if (_poll_cb)
  {
    _poll_cb(_poll_cb_arg, this);
  }

  return ERR_OK;
}

/////////////////////////////////////////////

void AsyncSSLClient::_dns_found(struct ip_addr *ipaddr)
{
  if (ipaddr && ipaddr->u_addr.ip4.addr)
  {
    connect(IPAddress(ipaddr->u_addr.ip4.addr), _connect_port, _pcb_secure);
  }
  else
  {
    if (_error_cb)
    {
      _error_cb(_error_cb_arg, this, -55);
    }

    if (_discard_cb)
    {
      _discard_cb(_discard_cb_arg, this);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
   Public Helper Methods
 * */

void AsyncSSLClient::stop()
{
  close(false);
}

/////////////////////////////////////////////

bool AsyncSSLClient::free()
{
  if (!_pcb)
  {
    return true;
  }

  if ( (_pcb->state == CLOSED) || (_pcb->state > ESTABLISHED) )
  {
    return true;
  }

  return false;
}

/////////////////////////////////////////////

size_t AsyncSSLClient::write(const char* data)
{
  if (data == NULL)
  {
    return 0;
  }

  return write(data, strlen(data));
}

/////////////////////////////////////////////

size_t AsyncSSLClient::write(const char* data, size_t size, uint8_t apiflags)
{
  size_t will_send = add(data, size, apiflags);

  if (!will_send || !send())
  {
    return 0;
  }

  return will_send;
}

/////////////////////////////////////////////

void AsyncSSLClient::setRxTimeout(uint32_t timeout)
{
  _rx_since_timeout = timeout;
}

/////////////////////////////////////////////

uint32_t AsyncSSLClient::getRxTimeout()
{
  return _rx_since_timeout;
}

/////////////////////////////////////////////

uint32_t AsyncSSLClient::getAckTimeout()
{
  return _ack_timeout;
}

/////////////////////////////////////////////

void AsyncSSLClient::setAckTimeout(uint32_t timeout)
{
  _ack_timeout = timeout;
}

/////////////////////////////////////////////

void AsyncSSLClient::setNoDelay(bool nodelay)
{
  if (!_pcb)
  {
    return;
  }

  if (nodelay)
  {
    tcp_nagle_disable(_pcb);
  }
  else
  {
    tcp_nagle_enable(_pcb);
  }
}

/////////////////////////////////////////////

bool AsyncSSLClient::getNoDelay()
{
  if (!_pcb)
  {
    return false;
  }

  return tcp_nagle_disabled(_pcb);
}

/////////////////////////////////////////////

uint16_t AsyncSSLClient::getMss()
{
  if (!_pcb)
  {
    return 0;
  }

  return tcp_mss(_pcb);
}

/////////////////////////////////////////////

uint32_t AsyncSSLClient::getRemoteAddress()
{
  if (!_pcb)
  {
    return 0;
  }

  return _pcb->remote_ip.u_addr.ip4.addr;
}

/////////////////////////////////////////////

uint16_t AsyncSSLClient::getRemotePort()
{
  if (!_pcb)
  {
    return 0;
  }

  return _pcb->remote_port;
}

/////////////////////////////////////////////

uint32_t AsyncSSLClient::getLocalAddress()
{
  if (!_pcb)
  {
    return 0;
  }

  return _pcb->local_ip.u_addr.ip4.addr;
}

/////////////////////////////////////////////

uint16_t AsyncSSLClient::getLocalPort()
{
  if (!_pcb)
  {
    return 0;
  }

  return _pcb->local_port;
}

/////////////////////////////////////////////

IPAddress AsyncSSLClient::remoteIP()
{
  return IPAddress(getRemoteAddress());
}

/////////////////////////////////////////////

uint16_t AsyncSSLClient::remotePort()
{
  return getRemotePort();
}

/////////////////////////////////////////////

IPAddress AsyncSSLClient::localIP()
{
  return IPAddress(getLocalAddress());
}

/////////////////////////////////////////////

uint16_t AsyncSSLClient::localPort()
{
  return getLocalPort();
}

/////////////////////////////////////////////

uint8_t AsyncSSLClient::state()
{
  if (!_pcb)
  {
    return 0;
  }

  return (_pcb->state);
}

/////////////////////////////////////////////

bool AsyncSSLClient::connected()
{
  if (!_pcb)
  {
    return false;
  }

  return (_pcb->state == ESTABLISHED);
}

/////////////////////////////////////////////

bool AsyncSSLClient::connecting()
{
  if (!_pcb)
  {
    return false;
  }

  return (_pcb->state > CLOSED && _pcb->state < ESTABLISHED);
}

/////////////////////////////////////////////

bool AsyncSSLClient::disconnecting()
{
  if (!_pcb)
  {
    return false;
  }

  return (_pcb->state > ESTABLISHED && _pcb->state < TIME_WAIT);
}

/////////////////////////////////////////////

bool AsyncSSLClient::disconnected()
{
  if (!_pcb)
  {
    return true;
  }

  return (_pcb->state == CLOSED || _pcb->state == TIME_WAIT);
}

/////////////////////////////////////////////

bool AsyncSSLClient::freeable()
{
  if (!_pcb)
  {
    return true;
  }

  return (_pcb->state == CLOSED || _pcb->state > ESTABLISHED);
}

/////////////////////////////////////////////

bool AsyncSSLClient::canSend()
{
  return space() > 0;
}

/////////////////////////////////////////////

const char * AsyncSSLClient::errorToString(int8_t error)
{
  switch (error)
  {
    case ERR_OK:
      return "OK";

    case ERR_MEM:
      return "Out of memory error";

    case ERR_BUF:
      return "Buffer error";

    case ERR_TIMEOUT:
      return "Timeout";

    case ERR_RTE:
      return "Routing problem";

    case ERR_INPROGRESS:
      return "Operation in progress";

    case ERR_VAL:
      return "Illegal value";

    case ERR_WOULDBLOCK:
      return "Operation would block";

    case ERR_USE:
      return "Address in use";

    case ERR_ALREADY:
      return "Already connected";

    case ERR_CONN:
      return "Not connected";

    case ERR_IF:
      return "Low-level netif error";

    case ERR_ABRT:
      return "Connection aborted";

    case ERR_RST:
      return "Connection reset";

    case ERR_CLSD:
      return "Connection closed";

    case ERR_ARG:
      return "Illegal argument";

    case -55:
      return "DNS failed";

    default:
      return "UNKNOWN";
  }
}

/////////////////////////////////////////////

const char * AsyncSSLClient::stateToString()
{
  switch (state())
  {
    case CLOSED:
      return "Closed";

    case LISTEN:
      return "Listen";

    case SYN_SENT:
      return "SYN Sent";

    case SYN_RCVD:
      return "SYN Received";

    case ESTABLISHED:
      return "Established";

    case FIN_WAIT_1:
      return "FIN Wait 1";

    case FIN_WAIT_2:
      return "FIN Wait 2";

    case CLOSE_WAIT:
      return "Close Wait";

    case CLOSING:
      return "Closing";

    case LAST_ACK:
      return "Last ACK";

    case TIME_WAIT:
      return "Time Wait";

    default:
      return "UNKNOWN";
  }
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
   Static Callbacks (LwIP C2C++ interconnect)
 * */

void AsyncSSLClient::_s_dns_found(const char * name, struct ip_addr * ipaddr, void * arg)
{
  reinterpret_cast<AsyncSSLClient*>(arg)->_dns_found(ipaddr);
}

/////////////////////////////////////////////

int8_t AsyncSSLClient::_s_poll(void * arg, struct tcp_pcb * pcb)
{
  return reinterpret_cast<AsyncSSLClient*>(arg)->_poll(pcb);
}

/////////////////////////////////////////////

int8_t AsyncSSLClient::_s_recv(void * arg, struct tcp_pcb * pcb, struct pbuf *pb, int8_t err)
{
  return reinterpret_cast<AsyncSSLClient*>(arg)->_recv(pcb, pb, err);
}

/////////////////////////////////////////////

int8_t AsyncSSLClient::_s_fin(void * arg, struct tcp_pcb * pcb, int8_t err)
{
  return reinterpret_cast<AsyncSSLClient*>(arg)->_fin(pcb, err);
}

/////////////////////////////////////////////

int8_t AsyncSSLClient::_s_lwip_fin(void * arg, struct tcp_pcb * pcb, int8_t err)
{
  return reinterpret_cast<AsyncSSLClient*>(arg)->_lwip_fin(pcb, err);
}

/////////////////////////////////////////////

int8_t AsyncSSLClient::_s_sent(void * arg, struct tcp_pcb * pcb, uint16_t len)
{
  return reinterpret_cast<AsyncSSLClient*>(arg)->_sent(pcb, len);
}

/////////////////////////////////////////////

void AsyncSSLClient::_s_error(void * arg, int8_t err)
{
  reinterpret_cast<AsyncSSLClient*>(arg)->_error(err);
}

/////////////////////////////////////////////

int8_t AsyncSSLClient::_s_connected(void * arg, void * pcb, int8_t err)
{
  return reinterpret_cast<AsyncSSLClient*>(arg)->_connected(pcb, err);
}

/////////////////////////////////////////////

void AsyncSSLClient::_s_data(void *arg, struct tcp_pcb *tcp, uint8_t * data, size_t len)
{
  AsyncSSLClient *c = reinterpret_cast<AsyncSSLClient*>(arg);

  if (c->_recv_cb)
    c->_recv_cb(c->_recv_cb_arg, c, data, len);
}

/////////////////////////////////////////////

void AsyncSSLClient::_s_handshake(void *arg, struct tcp_pcb *tcp, struct tcp_ssl_pcb* ssl)
{
  AsyncSSLClient *c  = reinterpret_cast<AsyncSSLClient*>(arg);
  c->_handshake_done = true;

  if (c->_connect_cb)
    c->_connect_cb(c->_connect_cb_arg, c);
}

/////////////////////////////////////////////

void AsyncSSLClient::_s_ssl_error(void *arg, struct tcp_pcb *tcp, int8_t err)
{
  reinterpret_cast<AsyncSSLClient*>(arg)->_ssl_error(err);
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
  Async TCP Server
*/

AsyncSSLServer::AsyncSSLServer(IPAddress addr, uint16_t port)
  : _port(port)
  , _addr(addr)
  , _noDelay(false)
  , _pcb(0)
  , _connect_cb(0)
  , _connect_cb_arg(0)
{}

/////////////////////////////////////////////

AsyncSSLServer::AsyncSSLServer(uint16_t port)
  : _port(port)
  , _addr((uint32_t) IPADDR_ANY)
  , _noDelay(false)
  , _pcb(0)
  , _connect_cb(0)
  , _connect_cb_arg(0)
{}

/////////////////////////////////////////////

AsyncSSLServer::~AsyncSSLServer()
{
  end();
}

/////////////////////////////////////////////

void AsyncSSLServer::onClient(AcConnectHandlerSSL cb, void* arg)
{
  _connect_cb     = cb;
  _connect_cb_arg = arg;
}

/////////////////////////////////////////////

void AsyncSSLServer::begin()
{
  if (_pcb)
  {
    return;
  }

  if (!_start_async_task())
  {
    ATCP_LOGERROR("begin: failed to start task");

    return;
  }

  int8_t err;

  _pcb = tcp_new_ip_type(IPADDR_TYPE_V4);

  if (!_pcb)
  {
    ATCP_LOGERROR("begin: NULL pcb");

    return;
  }

  ip_addr_t local_addr;

  local_addr.type            = IPADDR_TYPE_V4;
  local_addr.u_addr.ip4.addr = (uint32_t) _addr;

  err = _tcp_bind(_pcb, &local_addr, _port);

  if (err != ERR_OK)
  {
    _tcp_close(_pcb, -1);

    ATCP_LOGERROR1("begin: bind error, err =", err);

    return;
  }

  static uint8_t backlog = 5;

  _pcb = _tcp_listen_with_backlog(_pcb, backlog);

  if (!_pcb)
  {
    ATCP_LOGERROR("begin: NULL listen_pcb");

    return;
  }

  tcp_arg(_pcb, (void*) this);
  tcp_accept(_pcb, &_s_accept);
}

/////////////////////////////////////////////

void AsyncSSLServer::end()
{
  if (_pcb)
  {
    tcp_arg(_pcb, NULL);
    tcp_accept(_pcb, NULL);

    if (tcp_close(_pcb) != ERR_OK)
    {
      _tcp_abort(_pcb, -1);
    }

    _pcb = NULL;
  }
}

/////////////////////////////////////////////

//runs on LwIP thread
int8_t AsyncSSLServer::_accept(tcp_pcb* pcb, int8_t err)
{
  ATCP_HEXLOGDEBUG1("_accept: pcb =", (uint32_t) pcb);

  if (_connect_cb)
  {
    AsyncSSLClient *c = new AsyncSSLClient(pcb);

    if (c)
    {
      c->setNoDelay(_noDelay);

      return _tcp_accept(this, c);
    }
  }

  if (tcp_close(pcb) != ERR_OK)
  {
    tcp_abort(pcb);
  }

  ATCP_LOGERROR("_accept: fail");

  return ERR_OK;
}

/////////////////////////////////////////////

int8_t AsyncSSLServer::_accepted(AsyncSSLClient* client)
{
  if (_connect_cb)
  {
    _connect_cb(_connect_cb_arg, client);
  }

  return ERR_OK;
}

/////////////////////////////////////////////

void AsyncSSLServer::setNoDelay(bool nodelay)
{
  _noDelay = nodelay;
}

/////////////////////////////////////////////

bool AsyncSSLServer::getNoDelay()
{
  return _noDelay;
}

/////////////////////////////////////////////

uint8_t AsyncSSLServer::status()
{
  if (!_pcb)
  {
    return 0;
  }

  return _pcb->state;
}

/////////////////////////////////////////////

int8_t AsyncSSLServer::_s_accept(void * arg, tcp_pcb * pcb, int8_t err)
{
  return reinterpret_cast<AsyncSSLServer*>(arg)->_accept(pcb, err);
}

/////////////////////////////////////////////

int8_t AsyncSSLServer::_s_accepted(void *arg, AsyncSSLClient* client)
{
  return reinterpret_cast<AsyncSSLServer*>(arg)->_accepted(client);
}

/////////////////////////////////////////////

#endif /* ASYNCTCP_SSL_IML_H */
