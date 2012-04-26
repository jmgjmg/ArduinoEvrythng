/*
   Copyright 2012 Javier Montaner

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
   
 */


/*  
//WARNING: This function must be added to the PN532 library downloaded from SeeedStudio
void PN532::RFConfiguration(uint8_t mxRtyPassiveActivation) {
    pn532_packetbuffer[0] = PN532_RFCONFIGURATION;
    pn532_packetbuffer[1] = PN532_MAX_RETRIES; 
    pn532_packetbuffer[2] = 0xFF; // default MxRtyATR
    pn532_packetbuffer[3] = 0x01; // default MxRtyPSL
	pn532_packetbuffer[4] = mxRtyPassiveActivation;

	sendCommandCheckAck(pn532_packetbuffer, 5);
    // ignore response!
}
*/

// SPI digital PINs are HW hardcoded in both Ethernet and SeeedStudio NFC shields (SS - 10, MOSI - 11,  MISO - 12, SCK - 13
// Do not know why, but using a different SS PIN for NFC shield (PIN 5) and reusing the other three PINs(11,12,13) does not work. As soon as Ethernet shield is used (PIN 10 set to LOW)
// the NFC shield stops reading tags. 
// As a solution, using completely independent PINS for NFC shield (5, 3, 6, 9) works fine and no interference is detected between the two shields.
//
// Connect the Ethernet shield on top of Arduino Uno to get ICSP PINs 
// Both shields use the ICSP PINs for SPI protocol (rather than the actual digital PINs). Since the NFC shield ICSP PINS are unconnected, they need to be wired to the  
// appropriate digital PINs in the NFC shield.
//
// This is the required wiring:
//   * wire 5V and GND female PINS from the Ethernet shield to equivalent male PINs in NFC shield
//   * wire digital female PIN in the Ethernet shield to male digital PIN in NFC shield:
//            5  (Ethernet)  -> 10 (NFC)  SS
//
//   * wire female ICSP PINs of NFC shield with the appropriate female PINs of Ethernet shield:
//            MISO (ICSP)  -> 6 (Ethernet)
//            SCK  (ICSP)  -> 9 (Ethernet)
//            MOSI (ICSP)  -> 3 (Ethernet)
//
//   * finally wire female ICSP Vcc PIN of NFC shield with the 5V power female PIN of NFC shield
//            Vcc  (ICSP)  -> 5V (NFC) This is the 5 Volts power PIN (WARNING: it is NOT digital PIN 5 )

#include <PN532.h>
#include <SPI.h>
#include <Ethernet.h>

#define SCK  9  // 13
#define MOSI 3  // 11
#define SS 5
#define MISO 6  // 12  

PN532 nfc(SCK, MISO, MOSI, SS);

int greenLedPin =7;  
int redLedPin=8;

byte mac[] = {  0x90, 0xA2, 0xDA, 0x00, 0x00, 0x00 };
EthernetClient client;


unsigned long time;
unsigned long responseTime;

uint32_t tagId=0;
uint32_t xmitId=0;
char  tagIdString [11]= "1234567890";

uint32_t flowState =0;

#define STATE_IDDLE 0
#define STATE_SENDDATA 15
#define STATE_RESPONSE 20



void setup()
{
  pinMode (greenLedPin, OUTPUT);  
  pinMode (redLedPin, OUTPUT);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW);  
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  time = millis();
  Serial.begin(19200);
  Serial.println("Starting setup method...");
  digitalWrite(greenLedPin, HIGH);  
  //Initialise NFC reader
  nfc.begin();
  nfc.RFConfiguration(0x14); // default is 0xFF (try forever; ultimately it does time out but after a long while
                             // modifies NFC library to set up a timeout while searching for RFID tags
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    // stop
    for(;;);
  }    
  // ok, print received data!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  Serial.print("Supports "); Serial.println(versiondata & 0xFF, HEX);
  // configure board to read RFID tags and cards
  nfc.SAMConfig();
  digitalWrite(greenLedPin, LOW);  
 
  //Initialise Ethernet connection
  Serial.println("StartEthernet");
  digitalWrite(redLedPin, HIGH);    
  digitalWrite(5, HIGH); //SPI deselect RFID reader
  digitalWrite(10, LOW); //SPI select Ethernet
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // stop
    for(;;);
  }
  
  // give the Ethernet shield a second to initialize:
  delay(1000);
  digitalWrite(10, HIGH); //SPI deselect Ethernet

  Serial.println("NFC and Ethernet initialised OK");   
  digitalWrite(redLedPin, LOW);    
  delay(300);  
  digitalWrite(greenLedPin, HIGH);  
  digitalWrite(redLedPin, HIGH);          
  delay(300);
  digitalWrite(greenLedPin, LOW); 
  digitalWrite(redLedPin, LOW);  
  delay(300);  
  digitalWrite(greenLedPin, HIGH);  
  digitalWrite(redLedPin, HIGH);          
  delay(300);
  digitalWrite(greenLedPin, LOW); 
  digitalWrite(redLedPin, LOW);  
  flowState=STATE_IDDLE;
  delay(1000);
}



void loop()
{ 

  if ((millis()-time > 1000)&&(flowState==STATE_IDDLE)) {
      Serial.println("Checking NFC...");
    // look for Mifare type cards every second
    time=millis();
    digitalWrite(10, HIGH);//SPI deselect Ethernet    
    digitalWrite(4, HIGH);//SPI deselect Ethernet
    digitalWrite(5, LOW);//SPI select RFID reader
    Serial.println("Start NFC read");
    tagId = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A);
    Serial.println("End NFC read");
    digitalWrite(5, HIGH);//SPI deselect RFID reader
    digitalWrite(10, LOW);//SPI select Ethernet
    if (tagId != 0) 
    { 
      digitalWrite(greenLedPin, HIGH);  
      delay(100);
      digitalWrite(greenLedPin, LOW);    
      Serial.print("Read card #"); Serial.println(tagId);
      xmitId=0;
      uint32_t divisor= 1000000000;
      for (int i=0;i<10; i++){
         tagIdString [i]= char(48 + tagId/divisor);
         tagId=tagId%divisor;
         divisor /=10;      
      }
      time=millis();
      flowState=STATE_SENDDATA;
      return;
     }
  }   


  if (flowState==STATE_SENDDATA) {
      Serial.println("Connecting to server ...");
      // if you get a connection, report back via serial:
      if (client.connect("www.evrythng.net", 80)) {
          Serial.println("Connected. Making PUT http request");
          // Make a HTTP request:
          client.println("PUT http://evrythng.net/thngs/...yourThingIdHere.../properties/ReadTag HTTP/1.1");
          client.println("Content-Type: application/json");
          client.println("Accept: application/vnd.evrythng-v2+json");
          client.println("X-Evrythng-Token: ...yourAPITokenHere...");
          client.println("Host: evrythng.net");
          client.println("Content-Length: 45"); 
          client.println(""); 
          client.print("{\"key\": \"ReadTag\",\"value\":  \""); client.print(tagIdString); client.println("\"}"); 
          client.println(""); 
          client.println(""); 
          
          
          digitalWrite(redLedPin, HIGH);  
          delay(100);
          digitalWrite(redLedPin, LOW);           
          responseTime=millis();
          flowState=STATE_RESPONSE;
      }  else {
          Serial.println("Connection to server failed");
          digitalWrite(redLedPin, HIGH);  
          delay(500);
          digitalWrite(redLedPin, LOW);           
          flowState=STATE_IDDLE;
      } 

  }


  
  if (flowState== STATE_RESPONSE) {

    if (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
    if ((millis() - responseTime)>2000) {
        Serial.println();
        Serial.println("Closing connection to server");
        client.stop();
        digitalWrite(greenLedPin, HIGH);  
        digitalWrite(redLedPin, HIGH);          
        delay(100);
        digitalWrite(greenLedPin, LOW); 
        digitalWrite(redLedPin, LOW);               
        delay(100);
        digitalWrite(greenLedPin, HIGH);  
        digitalWrite(redLedPin, HIGH);          
        delay(100);
        digitalWrite(greenLedPin, LOW); 
        digitalWrite(redLedPin, LOW);               
        flowState=STATE_IDDLE;
    }
  }
  
}
 
 