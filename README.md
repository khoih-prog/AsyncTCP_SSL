# AsyncTCP_SSL Library

[![arduino-library-badge](https://www.ardu-badge.com/badge/AsyncTCP_SSL.svg?)](https://www.ardu-badge.com/AsyncTCP_SSL)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/AsyncTCP_SSL.svg)](https://github.com/khoih-prog/AsyncTCP_SSL/releases)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/AsyncTCP_SSL.svg)](http://github.com/khoih-prog/AsyncTCP_SSL/issues)


<a href="https://www.buymeacoffee.com/khoihprog6" title="Donate to my libraries using BuyMeACoffee"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Donate to my libraries using BuyMeACoffee" style="height: 50px !important;width: 181px !important;" ></a>
<a href="https://www.buymeacoffee.com/khoihprog6" title="Donate to my libraries using BuyMeACoffee"><img src="https://img.shields.io/badge/buy%20me%20a%20coffee-donate-orange.svg?logo=buy-me-a-coffee&logoColor=FFDD00" style="height: 20px !important;width: 200px !important;" ></a>

---
---

## Table of contents

* [Important Note for ESP32_S3](#Important-Note-for-ESP32_S3)
* [Important Change from v1.2.0](#Important-Change-from-v120)
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
* [Note for Platform IO using ESP32 LittleFS](#note-for-platform-io-using-esp32-littlefs) 
* [HOWTO Fix `Multiple Definitions` Linker Error](#howto-fix-multiple-definitions-linker-error)
* [Note for Platform IO using ESP32 LittleFS](#note-for-platform-io-using-esp32-littlefs)
* [HOWTO Use analogRead() with ESP32 running WiFi and/or BlueTooth (BT/BLE)](#howto-use-analogread-with-esp32-running-wifi-andor-bluetooth-btble)
  * [1. ESP32 has 2 ADCs, named ADC1 and ADC2](#1--esp32-has-2-adcs-named-adc1-and-adc2)
  * [2. ESP32 ADCs functions](#2-esp32-adcs-functions)
  * [3. ESP32 WiFi uses ADC2 for WiFi functions](#3-esp32-wifi-uses-adc2-for-wifi-functions)
* [Original documentation](#Original-documentation)
  * [AsyncSSLClient](#AsyncSSLClient)
* [Examples](#examples)
  * [1. multiFileProject](examples/multiFileProject)
* [Debug Terminal Output Samples](#debug-terminal-output-samples) 
  * [1. AsyncHTTPSRequest_ESP on ESP32_DEV](#1-AsyncHTTPSRequest_ESP-on-ESP32_DEV)
  * [2. AsyncHTTPSRequest_ESP on ESP32S2_DEV](#2-AsyncHTTPSRequest_ESP-on-ESP32S2_DEV)
  * [3. AsyncHTTPSRequest_ESP on ESP32C3_DEV](#3-AsyncHTTPSRequest_ESP-on-ESP32C3_DEV)
  * [4. AsyncHTTPSRequest_ESP_WiFiManager on ESP32_DEV](#4-AsyncHTTPSRequest_ESP_WiFiManager-on-ESP32_DEV)
  * [5. AsyncHTTPSRequest_ESP_Multi on ESP32S3_DEV](#5-AsyncHTTPSRequest_ESP_Multi-on-ESP32S3_DEV)
* [Debug](#debug)
* [Troubleshooting](#troubleshooting)
* [Issues](#issues)
* [TO DO](#to-do)
* [DONE](#done)
* [Contributions and Thanks](#contributions-and-thanks)
* [Contributing](#contributing)
* [License](#license)
* [Copyright](#copyright)

---
---

### Important Note for ESP32_S3

**Don't use `ESP32_S3` with core v2.0.4**. Check `already fixed` [ESP32-S3 Powercycling right after uploading a sketch using Arduino IDE and Arduino Core 2.0.4 #7165](https://github.com/espressif/arduino-esp32/issues/7165)

**ESP32_S3 is OK now with core v2.0.5**

---

### Important Change from v1.2.0

Please have a look at [HOWTO Fix `Multiple Definitions` Linker Error](#howto-fix-multiple-definitions-linker-error)

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

1. `ESP32` boards, such as ESP32_DEV, etc.
2. `ESP32_S2`-based boards, such as ESP32S2_DEV, ESP32_S2 Saola, etc.
3. `ESP32_C3`-based boards, such as ESP32C3_DEV, etc.
4. `ESP32_S3`-based boards, such as ESP32S3_DEV, etc., using ESP32 core `v2.0.3` or `v2.0.5+`


---
---

## Prerequisites

 1. [`Arduino IDE 1.8.19+` for Arduino](https://github.com/arduino/Arduino). [![GitHub release](https://img.shields.io/github/release/arduino/Arduino.svg)](https://github.com/arduino/Arduino/releases/latest)
 2. [`ESP32 Core 2.0.5+`](https://github.com/espressif/arduino-esp32) for ESP32-based boards. [![Latest release](https://img.shields.io/github/release/espressif/arduino-esp32.svg)](https://github.com/espressif/arduino-esp32/releases/latest/) for ESP32, ESP32_S2, ESP32_C3
 3. [`ESP32 Core 2.0.4`](https://github.com/espressif/arduino-esp32) can't be used for ESP32_S3-based boards. Check `already fixed` [ESP32-S3 Powercycling right after uploading a sketch using Arduino IDE and Arduino Core 2.0.4 #7165](https://github.com/espressif/arduino-esp32/issues/7165)

---
---

## Installation

### Use Arduino Library Manager

The best and easiest way is to use `Arduino Library Manager`. Search for [**AsyncTCP_SSL**](https://github.com/khoih-prog/AsyncTCP_SSL), then select / install the latest version.
You can also use this link [![arduino-library-badge](https://www.ardu-badge.com/badge/AsyncTCP_SSL.svg?)](https://www.ardu-badge.com/AsyncTCP_SSL) for more detailed instructions.

### Manual Install

Another way to install is to:

1. Navigate to [**AsyncTCP_SSL**](https://github.com/khoih-prog/AsyncTCP_SSL) page.
2. Download the latest release `AsyncTCP_SSL-main.zip`.
3. Extract the zip file to `AsyncTCP_SSL-main` directory 
4. Copy whole `AsyncTCP_SSL-main` folder to Arduino libraries' directory such as `~/Arduino/libraries/`.

### VS Code & PlatformIO

1. Install [VS Code](https://code.visualstudio.com/)
2. Install [PlatformIO](https://platformio.org/platformio-ide)
3. Install [**AsyncTCP_SSL** library](https://registry.platformio.org/libraries/khoih-prog/AsyncTCP_SSL) by using [Library Manager](https://registry.platformio.org/libraries/khoih-prog/AsyncTCP_SSL/installation). Search for **AsyncTCP_SSL** in [Platform.io Author's Libraries](https://platformio.org/lib/search?query=author:%22Khoi%20Hoang%22)
4. Use included [platformio.ini](platformio/platformio.ini) file from examples to ensure that all dependent libraries will installed automatically. Please visit documentation for the other options and examples at [Project Configuration File](https://docs.platformio.org/page/projectconf.html)

---
---


### Note for Platform IO using ESP32 LittleFS

In Platform IO, to fix the error when using [`LittleFS_esp32 v1.0`](https://github.com/lorol/LITTLEFS) for ESP32-based boards with ESP32 core v1.0.4- (ESP-IDF v3.2-), uncomment the following line

from

```cpp
//#define CONFIG_LITTLEFS_FOR_IDF_3_2   /* For old IDF - like in release 1.0.4 */
```

to

```cpp
#define CONFIG_LITTLEFS_FOR_IDF_3_2   /* For old IDF - like in release 1.0.4 */
```

It's advisable to use the latest [`LittleFS_esp32 v1.0.5+`](https://github.com/lorol/LITTLEFS) to avoid the issue.

Thanks to [Roshan](https://github.com/solroshan) to report the issue in [Error esp_littlefs.c 'utime_p'](https://github.com/khoih-prog/ESPAsync_WiFiManager/issues/28) 

---
---

### HOWTO Fix `Multiple Definitions` Linker Error

The current library implementation, using `xyz-Impl.h` instead of standard `xyz.cpp`, possibly creates certain `Multiple Definitions` Linker error in certain use cases.

You can include this `.hpp` file

```cpp
// Can be included as many times as necessary, without `Multiple Definitions` Linker Error
#include "AsyncTCP_SSL.hpp"     //https://github.com/khoih-prog/AsyncTCP_SSL
```

in many files. But be sure to use the following `.h` file **in just 1 `.h`, `.cpp` or `.ino` file**, which must **not be included in any other file**, to avoid `Multiple Definitions` Linker Error

```cpp
// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "AsyncTCP_SSL.h"       //https://github.com/khoih-prog/AsyncTCP_SSL
```

Check the new [**multiFileProject** example](examples/multiFileProject) for a `HOWTO` demo.

---
---

### Note for Platform IO using ESP32 LittleFS

In Platform IO, to fix the error when using [`LittleFS_esp32 v1.0`](https://github.com/lorol/LITTLEFS) for ESP32-based boards with ESP32 core v1.0.4- (ESP-IDF v3.2-), uncomment the following line

from

```cpp
//#define CONFIG_LITTLEFS_FOR_IDF_3_2   /* For old IDF - like in release 1.0.4 */
```

to

```cpp
#define CONFIG_LITTLEFS_FOR_IDF_3_2   /* For old IDF - like in release 1.0.4 */
```

It's advisable to use the latest [`LittleFS_esp32 v1.0.5+`](https://github.com/lorol/LITTLEFS) to avoid the issue.

Thanks to [Roshan](https://github.com/solroshan) to report the issue in [Error esp_littlefs.c 'utime_p'](https://github.com/khoih-prog/ESPAsync_WiFiManager/issues/28) 

---
---

### HOWTO Use analogRead() with ESP32 running WiFi and/or BlueTooth (BT/BLE)

Please have a look at [**ESP_WiFiManager Issue 39: Not able to read analog port when using the autoconnect example**](https://github.com/khoih-prog/ESP_WiFiManager/issues/39) to have more detailed description and solution of the issue.

#### 1.  ESP32 has 2 ADCs, named ADC1 and ADC2

#### 2. ESP32 ADCs functions

- ADC1 controls ADC function for pins **GPIO32-GPIO39**
- ADC2 controls ADC function for pins **GPIO0, 2, 4, 12-15, 25-27**

#### 3.. ESP32 WiFi uses ADC2 for WiFi functions

Look in file [**adc_common.c**](https://github.com/espressif/esp-idf/blob/master/components/driver/adc_common.c#L61)

> In ADC2, there're two locks used for different cases:
> 1. lock shared with app and Wi-Fi:
>    ESP32:
>         When Wi-Fi using the ADC2, we assume it will never stop, so app checks the lock and returns immediately if failed.
>    ESP32S2:
>         The controller's control over the ADC is determined by the arbiter. There is no need to control by lock.
> 
> 2. lock shared between tasks:
>    when several tasks sharing the ADC2, we want to guarantee
>    all the requests will be handled.
>    Since conversions are short (about 31us), app returns the lock very soon,
>    we use a spinlock to stand there waiting to do conversions one by one.
> 
> adc2_spinlock should be acquired first, then adc2_wifi_lock or rtc_spinlock.


- In order to use ADC2 for other functions, we have to **acquire complicated firmware locks and very difficult to do**
- So, it's not advisable to use ADC2 with WiFi/BlueTooth (BT/BLE).
- Use ADC1, and pins GPIO32-GPIO39
- If somehow it's a must to use those pins serviced by ADC2 (**GPIO0, 2, 4, 12, 13, 14, 15, 25, 26 and 27**), use the **fix mentioned at the end** of [**ESP_WiFiManager Issue 39: Not able to read analog port when using the autoconnect example**](https://github.com/khoih-prog/ESP_WiFiManager/issues/39) to work with ESP32 WiFi/BlueTooth (BT/BLE).

---
---


## Original documentation

For ESP32, check [AsyncTCP Library](https://github.com/me-no-dev/AsyncTCP)

This is a fully asynchronous SSL TCP library, aimed at enabling trouble-free, multi-connection network environment for Espressif's ESP32 MCUs.

### AsyncSSLClient

The base classes on which everything else is built. They expose all possible scenarios, but are really raw and require more skills to use.

---
---

### Examples

 1. [multiFileProject](examples/multiFileProject). **New**

---
---

### Debug Terminal Output Samples

#### 1. AsyncHTTPSRequest_ESP on ESP32_DEV

Following is the debug terminal when running example [AsyncHTTPSRequest_ESP](https://github.com/khoih-prog/AsyncHTTPSRequest_Generic/tree/main/examples/AsyncHTTPSRequest_ESP) on ESP32_DEV to demonstrate the operation of SSL Async HTTPS request, using [AsyncTCP_SSL Library](https://github.com/khoih-prog/AsyncTCP_SSL).

```cpp
Starting AsyncHTTPSRequest_ESP using ESP32_DEV
AsyncTCP_SSL v1.3.1
AsyncHTTPSRequest_Generic v2.1.2
Connecting to WiFi SSID: HueNet1
.......
AsyncHTTPSRequest @ IP : 192.168.2.133

**************************************
abbreviation: EDT
client_ip: aaa.bbb.ccc.ddd
datetime: 2022-09-18T01:35:32.451975-04:00
day_of_week: 0
day_of_year: 261
dst: true
dst_from: 2022-03-13T07:00:00+00:00
dst_offset: 3600
dst_until: 2022-11-06T06:00:00+00:00
raw_offset: -18000
timezone: America/Toronto
unixtime: 1663479332
utc_datetime: 2022-09-18T05:35:32.451975+00:00
utc_offset: -04:00
week_number: 37
**************************************
HHHHHH
**************************************
abbreviation: EDT
client_ip: aaa.bbb.ccc.ddd
datetime: 2022-09-18T01:36:31.992595-04:00
day_of_week: 0
day_of_year: 261
dst: true
dst_from: 2022-03-13T07:00:00+00:00
dst_offset: 3600
dst_until: 2022-11-06T06:00:00+00:00
raw_offset: -18000
timezone: America/Toronto
unixtime: 1663479391
utc_datetime: 2022-09-18T05:36:31.992595+00:00
utc_offset: -04:00
week_number: 37
```
---

#### 2. AsyncHTTPSRequest_ESP on ESP32S2_DEV

Following is the debug terminal when running example [AsyncHTTPSRequest_ESP](https://github.com/khoih-prog/AsyncHTTPSRequest_Generic/tree/main/examples/AsyncHTTPSRequest_ESP) on ESP32S2_DEV to demonstrate the operation of SSL Async HTTPS request, using [AsyncTCP_SSL Library](https://github.com/khoih-prog/AsyncTCP_SSL).

```cpp
Starting AsyncHTTPSRequest_ESP using ESP32S2_DEV
AsyncTCP_SSL v1.3.1
AsyncHTTPSRequest_Generic v2.1.2
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
datetime: 2022-09-18T01:46:31.858783-04:00
day_of_week: 0
day_of_year: 261
dst: true
dst_from: 2022-03-13T07:00:00+00:00
dst_offset: 3600
dst_until: 2022-11-06T06:00:00+00:00
raw_offset: -18000
timezone: America/Toronto
unixtime: 1663479991
utc_datetime: 2022-09-18T05:46:31.858783+00:00
utc_offset: -04:00
week_number: 37
**************************************
```

---

#### 3. AsyncHTTPSRequest_ESP on ESP32C3_DEV

Following is the debug terminal when running example [AsyncHTTPSRequest_ESP](https://github.com/khoih-prog/AsyncHTTPSRequest_Generic/tree/main/examples/AsyncHTTPSRequest_ESP) on ESP32C3_DEV to demonstrate the operation of SSL Async HTTPS request, using [AsyncTCP_SSL Library](https://github.com/khoih-prog/AsyncTCP_SSL).

```cpp
Starting AsyncHTTPSRequest_ESP using ESP32C3_DEV
AsyncTCP_SSL v1.3.1
AsyncHTTPSRequest_Generic v2.1.2
Connecting to WiFi SSID: HueNet1
.........
AsyncHTTPSRequest @ IP : 192.168.2.80
**************************************
abbreviation: EDT
client_ip: aaa.bbb.ccc.ddd
datetime: 2022-09-18T01:35:32.451975-04:00
day_of_week: 0
day_of_year: 261
dst: true
dst_from: 2022-03-13T07:00:00+00:00
dst_offset: 3600
dst_until: 2022-11-06T06:00:00+00:00
raw_offset: -18000
timezone: America/Toronto
unixtime: 1663479332
utc_datetime: 2022-09-18T05:35:32.451975+00:00
utc_offset: -04:00
week_number: 37
**************************************
```

---

#### 4. AsyncHTTPSRequest_ESP_WiFiManager on ESP32_DEV

Following is the debug terminal when running example [AsyncHTTPSRequest_ESP_WiFiManager](https://github.com/khoih-prog/AsyncHTTPSRequest_Generic/tree/main/examples/AsyncHTTPSRequest_ESP_WiFiManager) on ESP32_DEV to demonstrate the operation of SSL Async HTTPS request, using [AsyncTCP_SSL Library](https://github.com/khoih-prog/AsyncTCP_SSL), and [ESPAsync_WiFiManager Library](https://github.com/khoih-prog/ESPAsync_WiFiManager)

```cpp
Starting AsyncHTTPSRequest_ESP_WiFiManager using LittleFS on ESP32_DEV
ESPAsync_WiFiManager v1.14.1
AsyncTCP_SSL v1.3.1
AsyncHTTPSRequest_Generic v2.1.2
Stored: SSID = HueNet1, Pass = 12345678
Got stored Credentials. Timeout 120s
ConnectMultiWiFi in setup
After waiting 11.38 secs more in setup(), connection result is connected. Local IP: 192.168.2.232
H
**************************************
abbreviation: EDT
client_ip: aaa.bbb.ccc.ddd
datetime: 2022-09-18T01:36:31.992595-04:00
day_of_week: 0
day_of_year: 261
dst: true
dst_from: 2022-03-13T07:00:00+00:00
dst_offset: 3600
dst_until: 2022-11-06T06:00:00+00:00
raw_offset: -18000
timezone: America/Toronto
unixtime: 1663479391
utc_datetime: 2022-09-18T05:36:31.992595+00:00
utc_offset: -04:00
week_number: 37
**************************************
H
```

---

#### 5. AsyncHTTPSRequest_ESP_Multi on ESP32S3_DEV

Following is the debug terminal when running example [AsyncHTTPSRequest_ESP_Multi](https://github.com/khoih-prog/AsyncHTTPSRequest_Generic/tree/main/examples/AsyncHTTPSRequest_ESP_Multi) on **ESP32S3_DEV on ESP32 core v2.0.5** to demonstrate the operation of SSL Async HTTPS request, using [AsyncTCP_SSL Library](https://github.com/khoih-prog/AsyncTCP_SSL)


```cpp
Starting AsyncHTTPSRequest_ESP_Multi on ESP32S3_DEV
AsyncTCP_SSL v1.3.1
AsyncHTTPSRequest_Generic v2.1.2
Connecting to WiFi SSID: HueNet1
...
AsyncHTTPSRequest @ IP : 192.168.2.187

Sending request: https://worldtimeapi.org/api/timezone/Europe/Prague.txt

Sending request: https://www.myexternalip.com/raw
[AHTTPS] _onError handler SSL error = OK
[AHTTPS] _onError handler SSL error = OK

**************************************
[AHTTPS] Response Code =  HTTP OK

**************************************
aaa.bbb.ccc.ddd
**************************************

**************************************
[AHTTPS] Response Code =  HTTP OK

**************************************
abbreviation: CEST
client_ip: aaa.bbb.ccc.ddd
datetime: 2022-09-18T07:50:04.395849+02:00
day_of_week: 0
day_of_year: 261
dst: true
dst_from: 2022-03-27T01:00:00+00:00
dst_offset: 3600
dst_until: 2022-10-30T01:00:00+00:00
raw_offset: 3600
timezone: Europe/Prague
unixtime: 1663480204
utc_datetime: 2022-09-18T05:50:04.395849+00:00
utc_offset: +02:00
week_number: 37
**************************************

Sending request: https://worldtimeapi.org/api/timezone/America/Toronto.txt

**************************************
[AHTTPS] Response Code =  HTTP OK

**************************************
abbreviation: EDT
client_ip: aaa.bbb.ccc.ddd
datetime: 2022-09-18T01:50:05.382100-04:00
day_of_week: 0
day_of_year: 261
dst: true
dst_from: 2022-03-13T07:00:00+00:00
dst_offset: 3600
dst_until: 2022-11-06T06:00:00+00:00
raw_offset: -18000
timezone: America/Toronto
unixtime: 1663480205
utc_datetime: 2022-09-18T05:50:05.382100+00:00
utc_offset: -04:00
week_number: 37
**************************************
H
```

---
---

### Debug

Debug is enabled by default on Serial.

You can also change the debugging level `_ASYNC_TCP_SSL_LOGLEVEL_` from 0 to 4 in the sketch


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
3. Permit both HTTP and HTTPS

---

## DONE

 1. Add support to ESP32 using SSL
 2. Add Table of Contents
 3. Add debug feature
 4. Fix `multiple-definitions` linker error
 5. Add examples
 6. Remove hard-code if possible
 7. Improve debug messages by adding functions to display `error/state messages` instead of `cryptic error/state number`
 8. Add support to `ESP32_S3`, using ESP32 core `v2.0.3` or `v2.0.5+`. **Don't use `ESP32_S3` with core v2.0.4**. Check `already fixed` [ESP32-S3 Powercycling right after uploading a sketch using Arduino IDE and Arduino Core 2.0.4 #7165](https://github.com/espressif/arduino-esp32/issues/7165)
 9. Increase `ASYNC_QUEUE_LENGTH` to default **512 from 32** and make it user-configurable
10. Increase `ASYNC_TCP_PRIORITY` to default **10 from 3**, and make it user-configurable


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

- Copyright (c) 2016- Hristo Gochkov
- Copyright (c) 2019- Maarten Fremouw
- Copyright (c) 2019- Thorsten von Eicken
- Copyright (c) 2021- Khoi Hoang


