#ifndef KEEPIT_H
#define KEEPIT_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "Adafruit_FONA.h" // https://github.com/botletics/SIM7000-LTE-Shield/tree/master/Code


// RFID
#define RST_PIN         32          // Configurable, see typical pin layout above
#define SS_PIN          15          // Configurable, see typical pin layout above

// Buzzer
#define BUZZER_PIN      33          // Pin for buzzer (audio feedback)

RTC_DATA_ATTR bool locked; // locked when true, unlocked when false, stored in RTC memory to stay through deepsleep

// SMS

// Define *one* of the following lines:
#define SIMCOM_2G // SIM800/808/900/908, etc.
//#define SIMCOM_3G // SIM5320A/E
//#define SIMCOM_7000 // SIM7000A/C/E/G
//#define SIMCOM_7500 // SIM7500A/E

// For SIM7000 shield with ESP32
//#define FONA_PWRKEY 18
#define FONA_RST 5
#define FONA_TX 26 // ESP32 hardware serial RX2 (GPIO16)
#define FONA_RX 27 // ESP32 hardware serial TX2 (GPIO17)

// For ESP32 hardware serial
#include <HardwareSerial.h>
HardwareSerial fonaSS(1);

// Use this for 2G modules
#ifdef SIMCOM_2G
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

// Use this one for 3G modules
#elif defined(SIMCOM_3G)
Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

// Use this one for LTE CAT-M/NB-IoT modules (like SIM7000)
// Notice how we don't include the reset pin because it's reserved for emergencies on the LTE module!
#elif defined(SIMCOM_7000) || defined(SIMCOM_7500)
Adafruit_FONA_LTE fona = Adafruit_FONA_LTE();
#endif


uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
char replybuffer[255]; // this is a large buffer for replies
char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

void tone(byte pin, int freq);

void noTone(byte pin);

void rfid(void *parameter);

void sms(void *parameter);

void print_wakeup_reason();


#endif