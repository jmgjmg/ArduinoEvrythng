<article class="markdown-body entry-content" itemprop="mainContentOfPage"><h1>ArduinoEvrythng: Uploading Data to Evrythng from Arduino</h1>
<p>This is a sample Arduino application that uses the official Ethernet shield and SeeedStudio NFC shield. It has been tested with Arduino Uno (v1.0) but it should also work with other boards.</p>

<p>The setup method initialises the NFC and Ethernet shields. Then the loop method checks every one second for the presence of one RFID (MIFARE) tag. When one tag is detected, its Id is pushed to evrythng using the Ethernet library.</p>

<p>The code defines two LEDs to provide basic visual feedback. This can be removed or updated as required</p>

<h2>NFC Library</h2>
</p>The code uses the NFC library from SeeedStudio downloadable from <a href="http://www.seeedstudio.com/wiki/index.php?title=NFC_Shield">SeeedStudio Wiki</a>. This library needs to be updated with a new method <code>PN532::RFConfiguration</code> as explained <a href="#rfconfig">below</a>. This method updates the timeout used by the NFC chip (NXP PN532) to stop searching for new tags.</p>

<h2>Arduino Setup</h2>
<pre style="margin: 0 20px 0;"><code><a name="rfconfig"></a>/*  
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


#define SCK  9  // 13
#define MOSI 3  // 11
#define SS 5
#define MISO 6  // 12  

PN532 nfc(SCK, MISO, MOSI, SS);</code></pre>

<h2>Evrythng Setup</h2>
You have to retrieve your authentication token from https://evrythng.net/settings/tokens and update its value in your Arduino code: 
<pre><code style="margin: 0 20px 5px;">client.println("X-Evrythng-Token: ...yourAPITokenHere...");</code></pre>
You must also crate a new thng in evrytng with a property called ReadTag. Once created, you have to update its thngId value in your Arduino code: 
<pre><code style="margin: 0 20px 5px;">client.println("PUT http://evrythng.net/thngs/...yourThingIdHere.../properties/ReadTag HTTP/1.1");</code></pre>
<h2>License</h2>
<p>Copyright 2012 Javier Montaner</p>

<p>The ArduinoEvrythng software is licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at <a href="http://www.apache.org/licenses/LICENSE-2.0">http://www.apache.org/licenses/LICENSE-2.0</a></p>

<p>Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.</p>
</article>