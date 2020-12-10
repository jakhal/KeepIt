#ifndef KEEPIT_H
#define KEEPIT_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// RFID
#define RST_PIN         32          // Configurable, see typical pin layout above
#define SS_PIN          15          // Configurable, see typical pin layout above

RTC_DATA_ATTR bool locked; // locked when true, unlocked when false, stored in RTC memory to stay through deepsleep

#endif