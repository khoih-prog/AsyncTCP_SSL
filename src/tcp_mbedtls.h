/****************************************************************************************************************************
  tcp_mbedtls.h

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

#ifndef LWIPR_MBEDTLS_H
#define LWIPR_MBEDTLS_H

/////////////////////////////////////////////

#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"

/////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_TCP_SSL_INVALID_SSL           -101
#define ERR_TCP_SSL_INVALID_TCP           -102
#define ERR_TCP_SSL_INVALID_CLIENTFD      -103
#define ERR_TCP_SSL_INVALID_CLIENTFD_DATA -104
#define ERR_TCP_SSL_INVALID_DATA          -105

/////////////////////////////////////////////

struct tcp_pcb;
struct pbuf;
struct tcp_ssl_pcb;

/////////////////////////////////////////////

typedef void (* tcp_ssl_data_cb_t)(void *arg, struct tcp_pcb *tcp, uint8_t * data, size_t len);
typedef void (* tcp_ssl_handshake_cb_t)(void *arg, struct tcp_pcb *tcp, struct tcp_ssl_pcb* ssl);
typedef void (* tcp_ssl_error_cb_t)(void *arg, struct tcp_pcb *tcp, int8_t error);

/////////////////////////////////////////////

uint8_t tcp_ssl_has_client();
int     tcp_ssl_new_client(struct tcp_pcb *tcp, void *arg, const char* hostname, const char* root_ca,
                           const size_t root_ca_len,
                           const char* cli_cert, const size_t cli_cert_len, const char* cli_key, const size_t cli_key_len);
int     tcp_ssl_new_psk_client(struct tcp_pcb *tcp, void *arg, const char* psk_ident, const char* psk);
int     tcp_ssl_write(struct tcp_pcb *tcp, uint8_t *data, size_t len);
int     tcp_ssl_read(struct tcp_pcb *tcp, struct pbuf *p);
int     tcp_ssl_handshake_step(struct tcp_pcb *tcp);
int     tcp_ssl_free(struct tcp_pcb *tcp);
bool    tcp_ssl_has(struct tcp_pcb *tcp);
void    tcp_ssl_arg(struct tcp_pcb *tcp, void * arg);
void    tcp_ssl_data(struct tcp_pcb *tcp, tcp_ssl_data_cb_t arg);
void    tcp_ssl_handshake(struct tcp_pcb *tcp, tcp_ssl_handshake_cb_t arg);
void    tcp_ssl_err(struct tcp_pcb *tcp, tcp_ssl_error_cb_t arg);

/////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif // LWIPR_MBEDTLS_H

