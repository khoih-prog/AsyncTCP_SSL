/****************************************************************************************************************************
  multiFileProject.ino
  For ESP32

  AsyncTCP_SSL is a library for the ESP32

  Built by Khoi Hoang https://github.com/khoih-prog/AsyncTCP_SSL
  Licensed under MIT license
*****************************************************************************************************************************/

// To demo how to include files in multi-file Projects

#if !( defined(ESP32) )
  #error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

#define ASYNC_TCP_SSL_VERSION_MIN_TARGET      "AsyncTCP_SSL v1.3.1"
#define ASYNC_TCP_SSL_VERSION_MIN             1003001

#include "multiFileProject.h"

#include <WiFi.h>

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "AsyncTCP_SSL.h"

void setup()
{
  Serial.begin(115200);

  while (!Serial && millis() < 5000);

  Serial.println("\nStart multiFileProject on ");
  Serial.println(ARDUINO_BOARD);
  Serial.println(ASYNC_TCP_SSL_VERSION);

#if defined(ASYNC_TCP_SSL_VERSION_MIN)

  if (ASYNC_TCP_SSL_VERSION_INT < ASYNC_TCP_SSL_VERSION_MIN)
  {
    Serial.print("Warning. Must use this example on Version equal or later than : ");
    Serial.println(ASYNC_TCP_SSL_VERSION_MIN_TARGET);
  }

#endif

  Serial.print("You're OK now");
}

void loop()
{
  // put your main code here, to run repeatedly:
}
