/****************************************************************************************************************************
  AsyncTCP_SSL.hpp
   
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

#ifndef ASYNCTCP_SSL_HPP
#define ASYNCTCP_SSL_HPP

#if !defined(ESP32)
  #error This AsyncTCP_SSL library is supporting only ESP32
#endif

/////////////////////////////////////////////////

#define ASYNC_TCP_SSL_VERSION             "AsyncTCP_SSL v1.3.1"

#define ASYNC_TCP_SSL_VERSION_MAJOR       1
#define ASYNC_TCP_SSL_VERSION_MINOR       3
#define ASYNC_TCP_SSL_VERSION_PATCH       1

#define ASYNC_TCP_SSL_VERSION_INT         1003001

/////////////////////////////////////////////////

#include "Arduino.h"

#include "IPAddress.h"
#include "sdkconfig.h"
#include <functional>
#include <string>

// in ./libraries/WiFiClientSecure/src/ssl_client.h for ESP32
#include <ssl_client.h>

#include "tcp_mbedtls.h"

extern "C" 
{
  #include "freertos/FreeRTOS.h"
  #include "freertos/semphr.h"
  #include "lwip/pbuf.h"
}

#include "AsyncTCP_SSL_Debug.h"

/*****************************************************
// Defined in tools/sdk/esp32s3/include/lwip/lwip/src/include/lwip/tcpbase.h
enum tcp_state 
{
  CLOSED      = 0,
  LISTEN      = 1,
  SYN_SENT    = 2,
  SYN_RCVD    = 3,
  ESTABLISHED = 4,
  FIN_WAIT_1  = 5,
  FIN_WAIT_2  = 6,
  CLOSE_WAIT  = 7,
  CLOSING     = 8,
  LAST_ACK    = 9,
  TIME_WAIT   = 10
};
*****************************************************/

#define INVALID_CLOSED_SLOT        -1

//////////////////////////////////////////////////////////////////////////////////////////

//If core is not defined, then we are running in Arduino or PIO
#ifndef CONFIG_ASYNC_TCP_RUNNING_CORE
  #define CONFIG_ASYNC_TCP_RUNNING_CORE     		-1    //any available core
  #define CONFIG_ASYNC_TCP_USE_WDT          		1     //if enabled, adds between 33us and 200us per event
#endif

/////////////////////////////////////////////////

// Make ASYNC_QUEUE_LENGTH adjustable in sketch. Default 512
#ifndef ASYNC_QUEUE_LENGTH
  #define ASYNC_QUEUE_LENGTH 		512
#endif

// Make ASYNC_TCP_PRIORITY user-adjustable in sketch. Default 10, can't be less than 4
#if !defined(CONFIG_ASYNC_TCP_PRIORITY)
  #define CONFIG_ASYNC_TCP_PRIORITY 	(10)
#elif (CONFIG_ASYNC_TCP_PRIORITY < 4)
  #undef CONFIG_ASYNC_TCP_PRIORITY
  #define CONFIG_ASYNC_TCP_PRIORITY 	(4)
  #warning Adjust CONFIG_ASYNC_TCP_PRIORITY to 4
#endif

/////////////////////////////////////////////////

#define ASYNC_MAX_ACK_TIME      5000
#define ASYNC_WRITE_FLAG_COPY   0x01    //will allocate new buffer to hold the data while sending (else will hold reference to the data given)
#define ASYNC_WRITE_FLAG_MORE   0x02    //will not send PSH flag, meaning that there should be more data to be sent before the application should react.
#define SSL_HANDSHAKE_TIMEOUT   5000    // timeout to complete SSL handshake

//////////////////////////////////////////////////////////////////////////////////////////////

class AsyncSSLClient;

typedef std::function<void(void*, AsyncSSLClient*)> AcConnectHandlerSSL;
typedef std::function<void(void*, AsyncSSLClient*, size_t len, uint32_t time)> AcAckHandlerSSL;
typedef std::function<void(void*, AsyncSSLClient*, int8_t error)> AcErrorHandlerSSL;
typedef std::function<void(void*, AsyncSSLClient*, void *data, size_t len)> AcDataHandlerSSL;
typedef std::function<void(void*, AsyncSSLClient*, struct pbuf *pb)> AcPacketHandlerSSL;
typedef std::function<void(void*, AsyncSSLClient*, uint32_t time)> AcTimeoutHandlerSSL;

/////////////////////////////////////////////////

struct tcp_pcb;
struct ip_addr;

//////////////////////////////////////////////////////////////////////////////////////////////

class AsyncSSLClient 
{
  public:
    AsyncSSLClient(tcp_pcb* pcb = 0);
    virtual ~AsyncSSLClient();

    AsyncSSLClient & operator=(const AsyncSSLClient &other);
    AsyncSSLClient & operator+=(const AsyncSSLClient &other);

    bool operator==(const AsyncSSLClient &other);

    bool operator!=(const AsyncSSLClient &other) 
    {
      return !(*this == other);
    }

    bool    connect(IPAddress ip, uint16_t port, bool secure = false);
    bool    connect(const char* host, uint16_t port,  bool secure = false);
    void    setRootCa(const char* rootca, const size_t len);
    void    setClientCert(const char* cli_cert, const size_t len);
    void    setClientKey(const char* cli_key, const size_t len);
    void    setPsk(const char* psk_ident, const char* psk);

    void    close(bool now = false);
    void    stop();
    int8_t  abort();
    bool    free();

    bool    canSend();        //ack is not pending
    size_t  add(const char* data, size_t size, uint8_t apiflags = ASYNC_WRITE_FLAG_COPY); //add for sending
    bool    send();           //send all data added with the method above

    virtual size_t  space();  //space available in the TCP window
    //write equals add()+send()
    virtual size_t  write(const char* data);
    virtual size_t  write(const char* data, size_t size, uint8_t apiflags = ASYNC_WRITE_FLAG_COPY); //only when canSend() == true

    uint8_t state();
    bool    connecting();
    bool    connected();
    bool    disconnecting();
    bool    disconnected();
    bool    freeable();       //disconnected or disconnecting

    uint16_t  getMss();

    uint32_t  getRxTimeout();
    void      setRxTimeout(uint32_t timeout);     //no RX data timeout for the connection in seconds

    uint32_t  getAckTimeout();
    void      setAckTimeout(uint32_t timeout);    //no ACK timeout for the last sent packet in milliseconds

    void      setNoDelay(bool nodelay);
    bool      getNoDelay();

    uint32_t  getRemoteAddress();
    uint16_t  getRemotePort();
    uint32_t  getLocalAddress();
    uint16_t  getLocalPort();

    //compatibility
    IPAddress remoteIP();
    uint16_t  remotePort();
    IPAddress localIP();
    uint16_t  localPort();

    void    onConnect(AcConnectHandlerSSL cb, void* arg = 0);      //on successful connect
    void    onDisconnect(AcConnectHandlerSSL cb, void* arg = 0);   //disconnected
    void    onAck(AcAckHandlerSSL cb, void* arg = 0);              //ack received
    void    onError(AcErrorHandlerSSL cb, void* arg = 0);          //unsuccessful connect or error
    void    onData(AcDataHandlerSSL cb, void* arg = 0);            //data received (called if onPacket is not used)
    void    onPacket(AcPacketHandlerSSL cb, void* arg = 0);        //data received
    void    onTimeout(AcTimeoutHandlerSSL cb, void* arg = 0);      //ack timeout
    void    onPoll(AcConnectHandlerSSL cb, void* arg = 0);         //every 125ms when connected

    void    ackPacket(struct pbuf * pb);//ack pbuf from onPacket
    size_t  ack(size_t len); //ack data that you have not acked using the method below
    
    void ackLater() 
    {
      _ack_pcb = false;  //will not ack the current packet. Call from onData
    }

    const char *  errorToString(int8_t error);
    const char *  stateToString();

    //Do not use any of the functions below!
    static int8_t _s_poll(void *arg, struct tcp_pcb *tpcb);
    static int8_t _s_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *pb, int8_t err);
    static int8_t _s_fin(void *arg, struct tcp_pcb *tpcb, int8_t err);
    static int8_t _s_lwip_fin(void *arg, struct tcp_pcb *tpcb, int8_t err);
    static void   _s_error(void *arg, int8_t err);
    static int8_t _s_sent(void *arg, struct tcp_pcb *tpcb, uint16_t len);
    static int8_t _s_connected(void* arg, void* tpcb, int8_t err);
    static void   _s_dns_found(const char *name, struct ip_addr *ipaddr, void *arg);
    
    static void _s_data(void *arg, struct tcp_pcb *tcp, uint8_t * data, size_t len);
    static void _s_handshake(void *arg, struct tcp_pcb *tcp, struct tcp_ssl_pcb* ssl);
    static void _s_ssl_error(void *arg, struct tcp_pcb *tcp, int8_t err);

    int8_t      _recv(tcp_pcb* pcb, pbuf* pb, int8_t err);
    
    tcp_pcb * pcb() 
    {
      return _pcb;
    }

    //KH
    int8_t getClosed_Slot() 
    {
      return _closed_slot;
    }
    //////

  protected:
    tcp_pcb*        _pcb;
    std::string     _hostname;
    int8_t          _closed_slot;

    AcConnectHandlerSSL   _connect_cb;
    void*                 _connect_cb_arg;
    AcConnectHandlerSSL   _discard_cb;
    void*                 _discard_cb_arg;
    AcAckHandlerSSL       _sent_cb;
    void*                 _sent_cb_arg;
    AcErrorHandlerSSL     _error_cb;
    void*                 _error_cb_arg;
    AcDataHandlerSSL      _recv_cb;
    void*                 _recv_cb_arg;
    AcPacketHandlerSSL    _pb_cb;
    void*                 _pb_cb_arg;
    AcTimeoutHandlerSSL   _timeout_cb;
    void*                 _timeout_cb_arg;
    AcConnectHandlerSSL   _poll_cb;
    void*                 _poll_cb_arg;

    bool      _pcb_busy;
    uint32_t  _pcb_sent_at;
    bool      _ack_pcb;
    uint32_t  _rx_ack_len;
    uint32_t  _rx_last_packet;
    uint32_t  _rx_since_timeout;
    uint32_t  _ack_timeout;
    uint16_t  _connect_port;

    ////// SSL
    size_t  _root_ca_len;
    char*   _root_ca;
    size_t  _cli_cert_len;
    char*   _cli_cert;
    size_t  _cli_key_len;
    char*   _cli_key;
    bool    _pcb_secure;
    bool    _handshake_done;
    
    const char* _psk_ident;
    const char* _psk;
    //////

    int8_t  _close();
    void    _free_closed_slot();
    void    _allocate_closed_slot();
    int8_t  _connected(void* pcb, int8_t err);
    void    _error(int8_t err);
    int8_t  _poll(tcp_pcb* pcb);
    int8_t  _sent(tcp_pcb* pcb, uint16_t len);
    int8_t  _fin(tcp_pcb* pcb, int8_t err);
    int8_t  _lwip_fin(tcp_pcb* pcb, int8_t err);
    void    _dns_found(struct ip_addr *ipaddr);
    
    ////// SSL
    void    _ssl_error(int8_t err);
    //////

  public:
    AsyncSSLClient* prev;
    AsyncSSLClient* next;
};

typedef std::function<int(void* arg, const char *filename, uint8_t **buf)> AcSSlFileHandlerSSL;

//////////////////////////////////////////////////////////////////////////////////////////////

class AsyncSSLServer 
{
  public:
    AsyncSSLServer(IPAddress addr, uint16_t port);
    AsyncSSLServer(uint16_t port);
    ~AsyncSSLServer();
    
    void onClient(AcConnectHandlerSSL cb, void* arg);
    
    // Dummy, so it compiles with ESP Async WebServer library enabled.
    void    onSslFileRequest(AcSSlFileHandlerSSL cb, void* arg) {};
    void    beginSecure(const char *cert, const char *private_key_file, const char *password) {};

    void    begin();
    void    end();
    void    setNoDelay(bool nodelay);
    bool    getNoDelay();
    uint8_t status();

    //Do not use any of the functions below!
    static int8_t _s_accept(void *arg, tcp_pcb* newpcb, int8_t err);
    static int8_t _s_accepted(void *arg, AsyncSSLClient* client);

  protected:
    uint16_t  _port;
    IPAddress _addr;
    bool      _noDelay;
    
    tcp_pcb*              _pcb;
    
    AcConnectHandlerSSL   _connect_cb;   
    void*                 _connect_cb_arg;

    int8_t _accept(tcp_pcb* newpcb, int8_t err);
    int8_t _accepted(AsyncSSLClient* client);
};

//////////////////////////////////////////////////////////////////////////////////////////////

#endif /* ASYNCTCP_SSL_HPP */
