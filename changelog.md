# AsyncTCP_SSL Library

[![arduino-library-badge](https://www.ardu-badge.com/badge/AsyncTCP_SSL.svg?)](https://www.ardu-badge.com/AsyncTCP_SSL)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/AsyncTCP_SSL.svg)](https://github.com/khoih-prog/AsyncTCP_SSL/releases)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/AsyncTCP_SSL.svg)](http://github.com/khoih-prog/AsyncTCP_SSL/issues)


<a href="https://www.buymeacoffee.com/khoihprog6" title="Donate to my libraries using BuyMeACoffee"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Donate to my libraries using BuyMeACoffee" style="height: 50px !important;width: 181px !important;" ></a>
<a href="https://www.buymeacoffee.com/khoihprog6" title="Donate to my libraries using BuyMeACoffee"><img src="https://img.shields.io/badge/buy%20me%20a%20coffee-donate-orange.svg?logo=buy-me-a-coffee&logoColor=FFDD00" style="height: 20px !important;width: 200px !important;" ></a>

---
---

## Table of Contents

* [Changelog](#changelog)
  * [Releases v1.3.1](#Releases-v131)
  * [Releases v1.3.0](#Releases-v130)
  * [Releases v1.2.0](#Releases-v120)
  * [Releases v1.1.0](#Releases-v110)
  * [Initial Releases v1.0.0](#Initial-Releases-v100)

---
---

## Changelog

### Releases v1.3.1

1. Increase `ASYNC_QUEUE_LENGTH` to default 512 from 32 and make it user-configurable
2. Increase `ASYNC_TCP_PRIORITY` to default 10 from 3, and make it user-configurable


### Releases v1.3.0

1. Remove hard-code if possible
2. Improve debug messages by adding functions to display `error/state messages` instead of `cryptic error/state number`
3. Clean up
4. Add support to `ESP32_S3`, using ESP32 core `v2.0.3`. **Don't use `ESP32_S3` with core v2.0.4**. Check [ESP32-S3 Powercycling right after uploading a sketch using Arduino IDE and Arduino Core 2.0.4 #7165](https://github.com/espressif/arduino-esp32/issues/7165)

### Releases v1.2.0

1. Fix `multiple-definitions` linker error. Drop `src_cpp` and `src_h` directories
2. Add example [multiFileProject](examples/multiFileProject) to demo for multiple-file project.
3. Add `platformio.ini`

### Releases v1.1.0

1. Fix duplication bug when using `src_h`
2. Enable coexistence with AsyncTCP

### Initial Releases v1.0.0

1. Initial coding to support **ESP32**



