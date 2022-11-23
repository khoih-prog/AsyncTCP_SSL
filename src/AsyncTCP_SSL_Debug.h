/****************************************************************************************************************************
  AsyncTCP_SSL_Debug.h

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

#pragma once

#ifndef ASYNC_TCP_SSL_DEBUG_H
#define ASYNC_TCP_SSL_DEBUG_H

#ifdef ASYNC_TCP_SSL_DEBUG_PORT
  #define DBG_PORT_ATCP      ASYNC_TCP_SSL_DEBUG_PORT
#else
  #define DBG_PORT_ATCP      Serial
#endif

// Change _ASYNC_TCP_SSL_LOGLEVEL_ to set tracing and logging verbosity
// 0: DISABLED: no logging
// 1: ERROR: errors
// 2: WARN: errors and warnings
// 3: INFO: errors, warnings and informational (default)
// 4: DEBUG: errors, warnings, informational and debug

#ifndef _ASYNC_TCP_SSL_LOGLEVEL_
  #define _ASYNC_TCP_SSL_LOGLEVEL_       1
#endif

/////////////////////////////////////////////////////////

#define ATCP_PRINT_MARK      ATCP_PRINT("[ATCP] ")
#define ATCP_PRINT_SP        DBG_PORT_ATCP.print(" ")
#define ATCP_PRINT_SP0X      DBG_PORT_ATCP.print(" 0x")

#define ATCP_PRINT           DBG_PORT_ATCP.print
#define ATCP_PRINTLN         DBG_PORT_ATCP.println

/////////////////////////////////////////////////////////

#define ATCP_LOGERROR(x)         if(_ASYNC_TCP_SSL_LOGLEVEL_>0) { ATCP_PRINT_MARK; ATCP_PRINTLN(x); }
#define ATCP_LOGERROR0(x)        if(_ASYNC_TCP_SSL_LOGLEVEL_>0) { ATCP_PRINT(x); }
#define ATCP_LOGERROR1(x,y)      if(_ASYNC_TCP_SSL_LOGLEVEL_>0) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINTLN(y); }
#define ATCP_HEXLOGERROR1(x,y)   if(_ASYNC_TCP_SSL_LOGLEVEL_>0) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP0X; ATCP_PRINTLN(y, HEX); }
#define ATCP_LOGERROR2(x,y,z)    if(_ASYNC_TCP_SSL_LOGLEVEL_>0) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINTLN(z); }
#define ATCP_HEXLOGERROR2(x,y,z) if(_ASYNC_TCP_SSL_LOGLEVEL_>0) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP0X; ATCP_PRINT(y, HEX); ATCP_PRINT_SP0X; ATCP_PRINTLN(z, HEX); }
#define ATCP_LOGERROR3(x,y,z,w)  if(_ASYNC_TCP_SSL_LOGLEVEL_>0) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINT(z); ATCP_PRINT_SP; ATCP_PRINTLN(w); }
#define ATCP_LOGERROR5(x,y,z,w,xx,yy)  if(_ASYNC_TCP_SSL_LOGLEVEL_>0) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINT(z); ATCP_PRINT_SP; ATCP_PRINT(w); ATCP_PRINT_SP; ATCP_PRINT(xx); ATCP_PRINT_SP; ATCP_PRINTLN(yy); }

/////////////////////////////////////////////////////////

#define ATCP_LOGWARN(x)          if(_ASYNC_TCP_SSL_LOGLEVEL_>1) { ATCP_PRINT_MARK; ATCP_PRINTLN(x); }
#define ATCP_LOGWARN0(x)         if(_ASYNC_TCP_SSL_LOGLEVEL_>1) { ATCP_PRINT(x); }
#define ATCP_LOGWARN1(x,y)       if(_ASYNC_TCP_SSL_LOGLEVEL_>1) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINTLN(y); }
#define ATCP_HEXLOGWARN1(x,y)    if(_ASYNC_TCP_SSL_LOGLEVEL_>1) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP0X; ATCP_PRINTLN(y, HEX); }
#define ATCP_LOGWARN2(x,y,z)     if(_ASYNC_TCP_SSL_LOGLEVEL_>1) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINTLN(z); }
#define ATCP_HEXLOGWARN2(x,y,z)  if(_ASYNC_TCP_SSL_LOGLEVEL_>1) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP0X; ATCP_PRINT(y, HEX); ATCP_PRINT_SP0X; ATCP_PRINTLN(z, HEX); }
#define ATCP_LOGWARN3(x,y,z,w)   if(_ASYNC_TCP_SSL_LOGLEVEL_>1) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINT(z); ATCP_PRINT_SP; ATCP_PRINTLN(w); }
#define ATCP_LOGWARN5(x,y,z,w,xx,yy)  if(_ASYNC_TCP_SSL_LOGLEVEL_>1) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINT(z); ATCP_PRINT_SP; ATCP_PRINT(w); ATCP_PRINT_SP; ATCP_PRINT(xx); ATCP_PRINT_SP; ATCP_PRINTLN(yy); }

/////////////////////////////////////////////////////////

#define ATCP_LOGINFO(x)          if(_ASYNC_TCP_SSL_LOGLEVEL_>2) { ATCP_PRINT_MARK; ATCP_PRINTLN(x); }
#define ATCP_LOGINFO0(x)         if(_ASYNC_TCP_SSL_LOGLEVEL_>2) { ATCP_PRINT(x); }
#define ATCP_LOGINFO1(x,y)       if(_ASYNC_TCP_SSL_LOGLEVEL_>2) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINTLN(y); }
#define ATCP_HEXLOGINFO1(x,y)    if(_ASYNC_TCP_SSL_LOGLEVEL_>2) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP0X; ATCP_PRINTLN(y, HEX); }
#define ATCP_LOGINFO2(x,y,z)     if(_ASYNC_TCP_SSL_LOGLEVEL_>2) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINTLN(z); }
#define ATCP_HEXLOGINFO2(x,y,z)  if(_ASYNC_TCP_SSL_LOGLEVEL_>2) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP0X; ATCP_PRINT(y, HEX); ATCP_PRINT_SP0X; ATCP_PRINTLN(z, HEX); }
#define ATCP_LOGINFO3(x,y,z,w)   if(_ASYNC_TCP_SSL_LOGLEVEL_>2) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINT(z); ATCP_PRINT_SP; ATCP_PRINTLN(w); }
#define ATCP_LOGINFO5(x,y,z,w,xx,yy)  if(_ASYNC_TCP_SSL_LOGLEVEL_>2) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINT(z); ATCP_PRINT_SP; ATCP_PRINT(w); ATCP_PRINT_SP; ATCP_PRINT(xx); ATCP_PRINT_SP; ATCP_PRINTLN(yy); }

/////////////////////////////////////////////////////////

#define ATCP_LOGDEBUG(x)         if(_ASYNC_TCP_SSL_LOGLEVEL_>3) { ATCP_PRINT_MARK; ATCP_PRINTLN(x); }
#define ATCP_LOGDEBUG0(x)        if(_ASYNC_TCP_SSL_LOGLEVEL_>3) { ATCP_PRINT(x); }
#define ATCP_LOGDEBUG1(x,y)      if(_ASYNC_TCP_SSL_LOGLEVEL_>3) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINTLN(y); }
#define ATCP_HEXLOGDEBUG1(x,y)   if(_ASYNC_TCP_SSL_LOGLEVEL_>3) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP0X; ATCP_PRINTLN(y, HEX); }
#define ATCP_LOGDEBUG2(x,y,z)    if(_ASYNC_TCP_SSL_LOGLEVEL_>3) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINTLN(z); }
#define ATCP_HEXLOGDEBUG2(x,y,z) if(_ASYNC_TCP_SSL_LOGLEVEL_>3) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP0X; ATCP_PRINT(y, HEX); ATCP_PRINT_SP0X; ATCP_PRINTLN(z, HEX); }
#define ATCP_LOGDEBUG3(x,y,z,w)  if(_ASYNC_TCP_SSL_LOGLEVEL_>3) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINT(z); ATCP_PRINT_SP; ATCP_PRINTLN(w); }
#define ATCP_LOGDEBUG5(x,y,z,w,xx,yy)  if(_ASYNC_TCP_SSL_LOGLEVEL_>3) { ATCP_PRINT_MARK; ATCP_PRINT(x); ATCP_PRINT_SP; ATCP_PRINT(y); ATCP_PRINT_SP; ATCP_PRINT(z); ATCP_PRINT_SP; ATCP_PRINT(w); ATCP_PRINT_SP; ATCP_PRINT(xx); ATCP_PRINT_SP; ATCP_PRINTLN(yy); }

/////////////////////////////////////////////////////////

#endif    //ASYNC_TCP_SSL_DEBUG_H
