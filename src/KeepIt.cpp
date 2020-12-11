#include "KeepIt.h"

unsigned long delayStart = 0;     // the time the delay started
bool delayRunning = false;        // true if still waiting for delay to finish
unsigned long delayLength = 2000; // time to wait in ms

RTC_DATA_ATTR int bootCount = 0;

void setup()
{
  locked = 1;
  Serial.begin(115200); // Initialize serial communications with the PC
  //while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  
  // Deep Sleep
  
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  //Configure GPIO33 as ext0 wake up source for HIGH logic level
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35,1);


  SPI.begin(14, 12, 13, 15);         // Init SPI bus: CLK, MISO, MOSI, SS
  mfrc522.PCD_Init();                // Init MFRC522
  delay(7);                          // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details

  xTaskCreate(
      rfid,        /* Task function      */
      "RFID Task", /* Task name          */
      10000,       /* stack size, bytes  */
      NULL,
      1,
      NULL);
  xTaskCreate(
      sms,        /* Task function      */
      "SMS Task", /* Task name          */
      10000,       /* stack size, bytes  */
      NULL,
      1,
      NULL);
}

void sms(void *parameter)
{
  //  while (!Serial);

  pinMode(FONA_RST, OUTPUT);
  digitalWrite(FONA_RST, HIGH); // Default state

  // pinMode(FONA_PWRKEY, OUTPUT);

  // Turn on the module by pulsing PWRKEY low for a little bit
  // This amount of time depends on the specific module that's used
  //powerOn(); // See function definition at the very end of the sketch

  Serial.println(F("ESP32 Basic Test"));
  Serial.println(F("Initializing....(May take several seconds)"));

  // Note: The SIM7000A baud rate seems to reset after being power cycled (SIMCom firmware thing)
  // SIM7000 takes about 3s to turn on but SIM7500 takes about 15s
  // Press reset button if the module is still turning on and the board doesn't find it.
  // When the module is on it should communicate right after pressing reset
  
  // Start at default SIM7000 shield baud rate
  fonaSS.begin(115200, SERIAL_8N1, FONA_TX, FONA_RX); // baud rate, protocol, ESP32 RX pin, ESP32 TX pin

  Serial.println(F("Configuring to 9600 baud"));
  fonaSS.println("AT+IPR=9600"); // Set baud rate
  delay(100); // Short pause to let the command run
  fonaSS.begin(9600, SERIAL_8N1, FONA_TX, FONA_RX); // Switch to 9600
  if (! fona.begin(fonaSS)) {
    Serial.println(F("Couldn't find FONA"));
    while (1); // Don't proceed if it couldn't find the device
  }

  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case SIM800L:
      Serial.println(F("SIM800L")); break;
    case SIM800H:
      Serial.println(F("SIM800H")); break;
    case SIM808_V1:
      Serial.println(F("SIM808 (v1)")); break;
    case SIM808_V2:
      Serial.println(F("SIM808 (v2)")); break;
    case SIM5320A:
      Serial.println(F("SIM5320A (American)")); break;
    case SIM5320E:
      Serial.println(F("SIM5320E (European)")); break;
    case SIM7000A:
      Serial.println(F("SIM7000A (American)")); break;
    case SIM7000C:
      Serial.println(F("SIM7000C (Chinese)")); break;
    case SIM7000E:
      Serial.println(F("SIM7000E (European)")); break;
    case SIM7000G:
      Serial.println(F("SIM7000G (Global)")); break;
    case SIM7500A:
      Serial.println(F("SIM7500A (American)")); break;
    case SIM7500E:
      Serial.println(F("SIM7500E (European)")); break;
    default:
      Serial.println(F("???")); break;
  }

  // Print module IMEI number.
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

  // Set modem to full functionality
  fona.setFunctionality(1); // AT+CFUN=1

  // Configure a GPRS APN, username, and password.
  // You might need to do this to access your network's GPRS/data
  // network.  Contact your provider for the exact APN, username,
  // and password values.  Username and password are optional and
  // can be removed, but APN is required.
  //fona.setNetworkSettings(F("your APN"), F("your username"), F("your password"));
  //fona.setNetworkSettings(F("m2m.com.attz")); // For AT&T IoT SIM card
  //fona.setNetworkSettings(F("telstra.internet")); // For Telstra (Australia) SIM card - CAT-M1 (Band 28)
  fona.setNetworkSettings(F("internet.eplus.de"),F("eplus"),F("internet")); // For ALDI TALK SIM card

  // Optionally configure HTTP gets to follow redirects over SSL.
  // Default is not to follow SSL redirects, however if you uncomment
  // the following line then redirects over SSL will be followed.
  //fona.setHTTPSRedirect(true);

  /*
  // Other examples of some things you can set:
  fona.setPreferredMode(38); // Use LTE only, not 2G
  fona.setPreferredLTEMode(1); // Use LTE CAT-M only, not NB-IoT
  fona.setOperatingBand("CAT-M", 12); // AT&T uses band 12
//  fona.setOperatingBand("CAT-M", 13); // Verizon uses band 13
  fona.enableRTC(true);
  
  fona.enableSleepMode(true);
  fona.set_eDRX(1, 4, "0010");
  fona.enablePSM(true);

  // Set the network status LED blinking pattern while connected to a network (see AT+SLEDS command)
  fona.setNetLED(true, 2, 64, 3000); // on/off, mode, timer_on, timer_off
  fona.setNetLED(false); // Disable network status LED
  */ 

// Unlock the SIM with a PIN code
Serial.print(F("Unlocking SIM card: "));
if (! fona.unlockSIM("0707")) {
    Serial.println(F("Failed"));
} else {
    Serial.println(F("OK!"));
}
vTaskDelete(NULL);
}

void rfid(void *parameter)
{
  Serial.println("RFID task started.");
  for (;;)
  {

    if (mfrc522.PICC_IsNewCardPresent())
    {
      // Select one of the cards
      if (mfrc522.PICC_ReadCardSerial())
      {
        if (mfrc522.uid.uidByte[0] == 0x3d && // UID Byte 1
            mfrc522.uid.uidByte[1] == 0x51 && // UID Byte 2
            mfrc522.uid.uidByte[2] == 0x95 && // UID Byte 3
            mfrc522.uid.uidByte[3] == 0x62 && // UID Byte 4
            ((millis() - delayStart) >= delayLength))
        {
          locked = !locked;
          if (locked)
          {
            //digitalWrite(LED_BUILTIN, HIGH);
            tone(BUZZER_PIN, 500);
            delay(250);
            noTone(BUZZER_PIN);
            tone(BUZZER_PIN, 1000);
            delay(500);
            noTone(BUZZER_PIN);
            Serial.println("locked!");
            esp_deep_sleep_start();
          }
          else
          {
            //digitalWrite(LED_BUILTIN, LOW);
            tone(BUZZER_PIN, 1000);
            delay(250);
            noTone(BUZZER_PIN);
            tone(BUZZER_PIN, 500);
            delay(500);
            noTone(BUZZER_PIN);
            Serial.println("unlocked!");
          }
          //vTaskDelay(delayLength / portTICK_PERIOD_MS);
          delayStart = millis(); // start delay
        }
        else if ((millis() - delayStart) <= delayLength)
        {
        }
        else
        {
          tone(BUZZER_PIN, 250);
          delay(1000);
          noTone(BUZZER_PIN);
        }
      }
    }
    vTaskDelay(100);
    // Dump debug info about the card; PICC_HaltA() is automatically called
    //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
    //vTaskDelete(NULL);
  }
}

void loop()
{

vTaskDelay(100 /portTICK_PERIOD_MS);
}

void tone(byte pin, int freq)
{
  ledcSetup(0, 2000, 8);  // setup beeper
  ledcAttachPin(pin, 0);  // attach beeper
  ledcWriteTone(0, freq); // play tone
}

void noTone(byte pin)
{
  tone(pin, 0);
}

//Function that prints the reason by which ESP32 has been awaken from sleep
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad"); break;
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}