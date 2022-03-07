/*
* Arduino Wireless Communication Tutorial
*       Example 1 - Receiver Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(6, 7); // CE, CSN

const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
    char bus1[3], bus2[3], bus3[3];
    for (int i = 0; i < 9; i++) {
      if (i < 3) {
        bus1[i%3] = text[i]; 
      }
      else if (i > 2 || i < 6) {
        bus2[i%3] = text[i];
      }
      else if (i > 5) {
        bus3[i%3] = text[i];
      }
      Serial.printf("Bus%i: %c\n", i, bus1[i%3]);
    }
  }
}