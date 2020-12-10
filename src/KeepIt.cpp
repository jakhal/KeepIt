#include "KeepIt.h"



bool locked = 1; // locked when true, unlocked when false
unsigned long delayStart = 0; // the time the delay started
bool delayRunning = false; // true if still waiting for delay to finish
unsigned long delayLength = 2000; // time to wait in ms

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void tone(byte pin, int freq);
void noTone(byte pin);

void setup() {
  	Serial.begin(115200);		// Initialize serial communications with the PC
	//while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin(14,12,13,15);			// Init SPI bus: CLK, MISO, MOSI, SS
	mfrc522.PCD_Init();		// Init MFRC522
	delay(7);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
    xTaskCreate(
                    rfid,           /* Task function */
                    "RFID Task"
    )
}

void loop() {
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  if (mfrc522.uid.uidByte[0] == 0x3d && // UID Byte 1
     mfrc522.uid.uidByte[1] == 0x51 &&  // UID Byte 2
     mfrc522.uid.uidByte[2] == 0x95 &&  // UID Byte 3
     mfrc522.uid.uidByte[3] == 0x62 &&  // UID Byte 4
     ((millis() - delayStart) >= delayLength)) {
    locked = !locked ;
      if(locked){
      //digitalWrite(LED_BUILTIN, HIGH);
      tone(33, 500);
      delay(250);
      noTone(33);
      tone(33, 1000);
      delay(500);
      noTone(33);
      Serial.println("locked!");
        
      }
      else{
      //digitalWrite(LED_BUILTIN, LOW);
      tone(33, 1000);
      delay(250);
      noTone(33);
      tone(33, 500);
      delay(500);
      noTone(33);
      Serial.println("unlocked!");
      }
    delayStart = millis();   // start delay
  }
  else if ((millis() - delayStart) <= delayLength){

  }
  else
  {
    tone(33, 250);
      delay(1000);
      noTone(33);
  }
  
	// Dump debug info about the card; PICC_HaltA() is automatically called
	//mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  
}

//int playing = 0;
void tone(byte pin, int freq) {
  ledcSetup(0, 2000, 8); // setup beeper
  ledcAttachPin(pin, 0); // attach beeper
  ledcWriteTone(0, freq); // play tone
  //playing = pin; // store pin
}

void noTone(byte pin) {
  tone(pin, 0);
}
