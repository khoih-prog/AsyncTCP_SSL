# AsyncTCP_SSL 

[![arduino-library-badge](https://www.ardu-badge.com/badge/AsyncTCP_SSL.svg?)](https://www.ardu-badge.com/AsyncTCP_SSL)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/AsyncTCP_SSL.svg)](https://github.com/khoih-prog/AsyncTCP_SSL/releases)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/AsyncTCP_SSL.svg)](http://github.com/khoih-prog/AsyncTCP_SSL/issues)


---
---

## Table of contents

* [Table of contents](#table-of-contents)
* [Why do we need this AsyncTCP_SSL library](#why-do-we-need-this-AsyncTCP_SSL-library)
  * [Features](#features)
  * [Why Async is better](#why-async-is-better)
  * [Currently supported Boards](#currently-supported-boards)
* [Changelog](changelog.md)
* [Prerequisites](#prerequisites)
* [Installation](#installation)
  * [Use Arduino Library Manager](#use-arduino-library-manager)
  * [Manual Install](#manual-install)
  * [VS Code & PlatformIO](#vs-code--platformio)
* [Packages' Patches](#packages-patches)
  * [1. For Portenta_H7 boards using Arduino IDE in Linux](#1-for-portenta_h7-boards-using-arduino-ide-in-linux)
  * [2. To fix compile error relating to dns_gethostbyname and LwIP stack](#2-to-fix-compile-error-relating-to-dns_gethostbyname-and-lwip-stack)
* [Orignal documentation](#Orignal-documentation)
  * [AsyncClient and AsyncServer](#AsyncClient-and-AsyncServer)
  * [AsyncPrinter](#AsyncPrinter)
  * [AsyncTCPbuffer](#AsyncTCPbuffer)
  * [SyncClient](#SyncClient)
* [Debug Terminal Output Samples](#debug-terminal-output-samples) 
  * [1. Async_AdvancedWebServer on PORTENTA_H7_M7 using Ethernet](#1-Async_AdvancedWebServer-on-PORTENTA_H7_M7-using-Ethernet)
  * [2. Async_AdvancedWebServer on PORTENTA_H7_M7 using WiFi](#2-Async_AdvancedWebServer-on-PORTENTA_H7_M7-using-WiFi)
  * [3. AsyncHTTPRequest on PORTENTA_H7_M7 using WiFi](#3-AsyncHTTPRequest-on-PORTENTA_H7_M7-using-WiFi)
  * [4. AsyncHTTPRequest on PORTENTA_H7_M7 using Ethernet](#4-AsyncHTTPRequest-on-PORTENTA_H7_M7-using-Ethernet)
* [Debug](#debug)
* [Troubleshooting](#troubleshooting)
* [Releases](#releases)
* [Issues](#issues)
* [TO DO](#to-do)
* [DONE](#done)
* [Contributions and Thanks](#contributions-and-thanks)
* [Contributing](#contributing)
* [License](#license)
* [Copyright](#copyright)

---
---

### Why do we need this [AsyncTCP_SSL library](https://github.com/khoih-prog/AsyncTCP_SSL)

#### Features

This library is based on, modified from:

1. [Hristo Gochkov's AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
2. [Maarten Fremouw's AsyncTCP](https://github.com/fremouw/AsyncTCP).
3. [Thorsten von Eicken's AsyncTCP](https://github.com/tve/AsyncTCP)

to apply the better and faster **asynchronous** feature of the **powerful** [AsyncTCP Library](https://github.com/me-no-dev/AsyncTCP) with SSL, and will be the base for future and more advanced Async libraries for ESP32, such as AsyncSSLWebServer, AsyncHTTPSRequest, etc.


#### Why Async is better

- Using asynchronous network means that you can handle **more than one connection at the same time**
- **You are called once the request is ready and parsed**
- When you send the response, you are **immediately ready** to handle other connections while the server is taking care of sending the response in the background
- **Speed is OMG**
- **Easy to use API, HTTP Basic and Digest MD5 Authentication (default), ChunkedResponse**
- Easily extensible to handle **any type of content**
- Supports Continue 100
- **Async WebSocket plugin offering different locations without extra servers or ports**
- Async EventSource (Server-Sent Events) plugin to send events to the browser
- URL Rewrite plugin for conditional and permanent url rewrites
- ServeStatic plugin that supports cache, Last-Modified, default index and more
- Simple template processing engine to handle templates


### Currently supported Boards

1. ESP32 boards, such as ESP32_DEV, etc.
2. ESP32S2-based boards, such as ESP32S2_DEV, ESP32_S2 Saola, etc.
3. ESP32C3-based boards, such as ESP32C3_DEV, etc.


---
---

## Prerequisites

 1. [`Arduino IDE 1.8.16+` for Arduino](https://www.arduino.cc/en/Main/Software)
 2. [`ESP32 Core 2.0.0+`](https://github.com/espressif/arduino-esp32) for ESP32-based boards. [![Latest release](https://img.shields.io/github/release/espressif/arduino-esp32.svg)](https://github.com/espressif/arduino-esp32/releases/latest/)

---
---

## Installation

### Use Arduino Library Manager

The best and easiest way is to use `Arduino Library Manager`. Search for [**AsyncTCP_SSL**](https://github.com/khoih-prog/AsyncTCP_SSL), then select / install the latest version.
You can also use this link [![arduino-library-badge](https://www.ardu-badge.com/badge/AsyncTCP_SSL.svg?)](https://www.ardu-badge.com/AsyncTCP_SSL) for more detailed instructions.

### Manual Install

Another way to install is to:

1. Navigate to [**AsyncTCP_SSL**](https://github.com/khoih-prog/AsyncTCP_SSL) page.
2. Download the latest release `AsyncTCP_SSL-master.zip`.
3. Extract the zip file to `AsyncTCP_SSL-master` directory 
4. Copy whole `AsyncTCP_SSL-master` folder to Arduino libraries' directory such as `~/Arduino/libraries/`.

### VS Code & PlatformIO

1. Install [VS Code](https://code.visualstudio.com/)
2. Install [PlatformIO](https://platformio.org/platformio-ide)
3. Install [**AsyncTCP_SSL** library](https://platformio.org/lib/show/xxxxx/AsyncTCP_SSL) by using [Library Manager](https://platformio.org/lib/show/xxxxx/AsyncTCP_SSL/installation). Search for **AsyncTCP_SSL** in [Platform.io Author's Libraries](https://platformio.org/lib/search?query=author:%22Khoi%20Hoang%22)
4. Use included [platformio.ini](platformio/platformio.ini) file from examples to ensure that all dependent libraries will installed automatically. Please visit documentation for the other options and examples at [Project Configuration File](https://docs.platformio.org/page/projectconf.html)

---
---


## Orignal documentation

For ESP32, check [AsyncTCP Library](https://github.com/me-no-dev/AsyncTCP)

This is a fully asynchronous SSL TCP library, aimed at enabling trouble-free, multi-connection network environment for Espressif's ESP32 MCUs.

### AsyncSSLClient

The base classes on which everything else is built. They expose all possible scenarios, but are really raw and require more skills to use.


---
---

### Debug Terminal Output Samples

#### 1. AsyncHTTPSRequest_ESP on ESP32_DEV

Following is the debug terminal when running example [AsyncHTTPSRequest_ESP](https://github.com/khoih-prog/AsyncHTTPSRequest_Generic/tree/main/examples/AsyncHTTPSRequest_ESP) on ESP32_DEV to demonstrate the operation of SSL Async HTTPS request, based on this [AsyncTCP_SSL Library](https://github.com/khoih-prog/AsyncTCP_SSL).

```
Starting AsyncHTTPSRequest_ESP using ESP32_DEV
AsyncTCP_SSL v1.0.0
AsyncHTTPSRequest_Generic v1.0.0
Connecting to WiFi SSID: HueNet1
.......
AsyncHTTPSRequest @ IP : 192.168.2.78

**************************************
abbreviation: EDT
client_ip: aaa.bbb.ccc.ddd
datetime: 2021-10-21T16:05:03.170256-04:00
day_of_week: 4
day_of_year: 294
dst: true
dst_from: 2021-03-14T07:00:00+00:00
dst_offset: 3600
dst_until: 2021-11-07T06:00:00+00:00
raw_offset: -18000
timezone: America/Toronto
unixtime: 1634846703
utc_datetime: 2021-10-21T20:05:03.170256+00:00
utc_offset: -04:00
week_number: 42
**************************************
HHHHHH
**************************************
abbreviation: EDT
client_ip: aaa.bbb.ccc.ddd
datetime: 2021-10-21T16:06:00.828056-04:00
day_of_week: 4
day_of_year: 294
dst: true
dst_from: 2021-03-14T07:00:00+00:00
dst_offset: 3600
dst_until: 2021-11-07T06:00:00+00:00
raw_offset: -18000
timezone: America/Toronto
unixtime: 1634846760
utc_datetime: 2021-10-21T20:06:00.828056+00:00
utc_offset: -04:00
week_number: 42
**************************************
```
---

#### 2. AsyncHTTPSRequest_ESP on ESP32S2_DEV

Following is the debug terminal when running example [AsyncHTTPSRequest_ESP](https://github.com/khoih-prog/AsyncHTTPSRequest_Generic/tree/main/examples/AsyncHTTPSRequest_ESP) on ESP32S2_DEV to demonstrate the operation of SSL Async HTTPS request, based on this [AsyncTCP_SSL Library](https://github.com/khoih-prog/AsyncTCP_SSL).

```

Starting AsyncHTTPSRequest_ESP using ESP32S2_DEV
AsyncTCP_SSL v1.0.0
AsyncHTTPSRequest_Generic v1.0.0
Connecting to WiFi SSID: HueNet1
.......
AsyncHTTPSRequest @ IP : 192.168.2.79
[ATCP] _handle_async_event: LWIP_TCP_DNS = 0x3FFE4E24
[ATCP] _handle_async_event: LWIP_TCP_DNS, name = worldtimeapi.org , IP = 213.188.196.246
[ATCP] _handle_async_event: LWIP_TCP_CONNECTED = 0x3FFE4E24 0x3FFE5024
[ATCP] _handle_async_event: LWIP_TCP_CONNECTED =  0
[ATCP] _handle_async_event: LWIP_TCP_SENT = 0x3FFE5024
[ATCP] _sent: len = 305
[ATCP] _handle_async_event: LWIP_TCP_RECV = 0x3FFE5024
[ATCP] _recv: tot_len = 1436
[ATCP] _handle_async_event: LWIP_TCP_RECV = 0x3FFE5024
[ATCP] _recv: tot_len = 1436
[ATCP] _handle_async_event: LWIP_TCP_RECV = 0x3FFE5024
[ATCP] _recv: tot_len = 1242
[ATCP] _handle_async_event: LWIP_TCP_SENT = 0x3FFE5024
[ATCP] _sent: len = 107
[ATCP] _handle_async_event: LWIP_TCP_SENT = 0x3FFE5024
[ATCP] _sent: len = 6
[ATCP] _handle_async_event: LWIP_TCP_SENT = 0x3FFE5024
[ATCP] _sent: len = 45
[ATCP] _handle_async_event: LWIP_TCP_RECV = 0x3FFE5024
[ATCP] _recv: tot_len = 51
[ATCP] _handle_async_event: LWIP_TCP_SENT = 0x3FFE5024
[ATCP] _sent: len = 106
[ATCP] _handle_async_event: LWIP_TCP_RECV = 0x3FFE5024
[ATCP] _recv: tot_len = 1016

**************************************
abbreviation: EDT
client_ip: aaa.bbb.ccc.ddd
datetime: 2021-10-21T23:19:43.835205-04:00
day_of_week: 4
day_of_year: 294
dst: true
dst_from: 2021-03-14T07:00:00+00:00
dst_offset: 3600
dst_until: 2021-11-07T06:00:00+00:00
raw_offset: -18000
timezone: America/Toronto
unixtime: 1634872783
utc_datetime: 2021-10-22T03:19:43.835205+00:00
utc_offset: -04:00
week_number: 42
**************************************

```

---
---

### Debug

Debug is enabled by default on Serial.

You can also change the debugging level `_ASYNC_TCP_SSL_LOGLEVEL_` from 0 to 4 in the sketch (if using `src_h`) or in the library `cpp` file (if using `src_cpp`)

```cpp
#define _ASYNC_TCP_SSL_LOGLEVEL_     1
```

---

### Troubleshooting

If you get compilation errors, more often than not, you may need to install a newer version of the core for Arduino boards.

Sometimes, the library will only work if you update the board core to the latest version because I am using newly added functions.


---
---

### Issues

Submit issues to: [AsyncTCP_SSL issues](https://github.com/khoih-prog/AsyncTCP_SSL/issues)

---

## TO DO

1. Search for bug and improvement.
2. Similar Async SSL libraries for ESP8266, STM32, Portenta_H7 and many other boards

---

## DONE

1. Add support to ESP32 using SSL
2. Add Table of Contents
3. Add debug feature

---
---

### Contributions and Thanks

Many thanks for everyone for bug reporting, new feature suggesting, testing and contributing to the development of this library.

### Contributions and Thanks

1. Based on and modified from [Hristo Gochkov's AsyncTCP](https://github.com/me-no-dev/AsyncTCP). Many thanks to [Hristo Gochkov](https://github.com/me-no-dev) for great [AsyncTCP Library](https://github.com/me-no-dev/AsyncTCP)
2. Based on [Maarten Fremouw's AsyncTCP Library](https://github.com/fremouw/AsyncTCP).
3. Based on [Thorsten von Eicken's AsyncTCP Library](https://github.com/tve/AsyncTCP).

<table>
  <tr>
    <td align="center"><a href="https://github.com/me-no-dev"><img src="https://github.com/me-no-dev.png" width="100px;" alt="me-no-dev"/><br /><sub><b>⭐️⭐️ Hristo Gochkov</b></sub></a><br /></td>
     <td align="center"><a href="https://github.com/fremouw"><img src="https://github.com/fremouw.png" width="100px;" alt="fremouw"/><br /><sub><b>Maarten Fremouw</b></sub></a><br /></td>
     <td align="center"><a href="https://github.com/tve"><img src="https://github.com/tve.png" width="100px;" alt="tve"/><br /><sub><b>Thorsten von Eicken</b></sub></a><br /></td>
  </tr> 
</table>

---

## Contributing

If you want to contribute to this project:

- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

---

### License

- The library is licensed under [LGPLv3](https://github.com/khoih-prog/AsyncTCP_SSL/blob/main/LICENSE)

---

## Copyright

Copyright 2016- Hristo Gochkov
Copyright 2019- Maarten Fremouw
Copyright 2019- Thorsten von Eicken
Copyright 2021- Khoi Hoang


