#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
#include "Wire.h"
#include <I2CKeyPad.h>
#include <GFXcanvas16T.h>
#include <SSD1283A.h>

#define MODEL SSD1283A
#define CS 3
#define A0 1
#define RST 2

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

SSD1283A lcd(CS,A0,RST,-1);//cd,cd,rst,led
RF24 radio(6, 7); // CE, CSN
I2CKeyPad keyPad(0x20);

const byte address[6] = "00001";
char keys[] = "D#0*C987B654A321NF";  // N = Nokey, F = Fail
int w = lcd.width();
int h = lcd.height();

void setup() {
  Serial.begin(115200);
  Wire.begin(400000);
  lcd.init();
  lcd.fillScreen(BLACK);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  if (keyPad.begin() == false)
  {
    Serial.println("\nERROR: cannot communicate to keypad.\nPlease reboot.\n");
    while(1);
  }
  keyPad.loadKeyMap(keys);
}

bool keyPress() {
  while (keyPad.isPressed()) {
    delay(50);
  }
}

char radioIn() {
  recheck:
  if (radio.available()) {
    char text = NULL;
    radio.read(&text, sizeof(text));
    if (text == NULL) {
      delay(500);
      goto recheck;
    }
    return text;
  }
}

int keyIn() {
  rekey:
  uint8_t idx = keyPad.getKey();
  if ( (keys[idx] != 'N') && (keys[idx] != 'F')) {
    return keys[idx];
  }
  goto rekey;
}

int busStore(char* nums, int index, int* bus) {
  String temp = String(nums[0]) + String(nums[1]) + String(nums[2]);
  int busNum = temp.toInt();
  bus[index] = busNum;
}

#define isNum 0b00001
#define curLn 0b00010
#define manX  0b00100
#define manY  0b01000
#define manL  0b10000

void csPrint(String str, int txt, uint8_t opt = 0b00000, int mX = 0, int mY = 0, int mL = 0) {
  lcd.setTextSize(txt); 
  int len = str.length();
  Serial.print("Length: "); Serial.println(len);
  if (opt & manL) {
    Serial.println("Manual length.");
    len = mL;
  }
  double px = len * 5.25 * txt;
  Serial.print(px);
  if (opt & manX) {
    Serial.println("Manual X.");
    px = mX;
  }
  else {
    px = (w-px)/2;
    Serial.print("Center. ");
    Serial.println(px);
  }
  if (opt & curLn) {
    Serial.println("Current line.");
    lcd.setCursor(px, lcd.getCursorY());
  }
  else if (opt & manY) {
    Serial.println("Manual Y.");
    lcd.setCursor(px, mY);
  }
  else {
    Serial.println("Y0");
    lcd.setCursor(px, 0);
  }
  if (opt & isNum) {
    lcd.setCursor(lcd.getCursorX() + 7, lcd.getCursorY());
  }
  lcd.print(str);
}

void loop() {
  int INIT_PIN = digitalRead(0);
  lcd.fillScreen(RED);
  delay(1000);
  int bus[3] = {0, 0, 0};
  renum:
  lcd.fillScreen(BLACK);
  bool init = (INIT_PIN == HIGH) ? true : false; // If pin 0 is pulled high, skip initialization.
  Serial.println(init);
  while (!init) { // lcd positions are 15, 54, 93
    char busNum[3] = {NULL,NULL,NULL};
    csPrint("Bus Number\n", 2);
    busNum[0] = keyIn();
    keyPress();
    csPrint(String(busNum[0]),4,0b00110,15);
    busNum[1] = keyIn();
    keyPress();
    csPrint(String(busNum[1]),4,0b00110,54);
    busNum[2] = keyIn();
    keyPress();
    csPrint(String(busNum[2]),4,0b00110,93);
    switch(keyIn()) {
      case 'A':
      init = true;
      if (bus[0] == 0) {
        busStore(busNum, 0, bus);
      }
      else if (bus[1] == 0) {
        busStore(busNum, 1, bus);
      }
      else if (bus[2] == 0) {
        busStore(busNum, 2, bus);
      }
      
      keyPress();
      break;
      case 'B':
      if (bus[0] == 0) {
        busStore(busNum, 0, bus);
      }
      else if (bus[1] == 0) {
        busStore(busNum, 1, bus);
      }
      else if (bus[2] == 0) {
        busStore(busNum, 2, bus);
      }
      keyPress();
      goto renum;
      break;
      default:
      Serial.println("Error: No key response.");
      break;
    }
  }
  if (INIT_PIN == HIGH) {
    Serial.println("Initialization skipped.");
    bus[0] = 123;
    bus[1] = 456; // Used while testing without keypad.
    bus[2] = 789;
  }
  lcd.fillScreen(GREEN);
  delay(1000);
  lcd.fillScreen(BLACK);
  csPrint("Transmitting:\n",1);
  if (bus[0] != 0) {
    csPrint(String(bus[0]+String("\n")),3,0b00011);
    if (bus[1] != 0) {
    csPrint(String(bus[1]+String("\n")),3,0b01001,0,lcd.getCursorY());
      if (bus[2] != 0) {
    csPrint(String(bus[2]+String("\n")),3,0b01001,0,lcd.getCursorY());
      }
    }
  }
  csPrint(String("~End List~"),1,0b01000,0,lcd.getCursorY());
  String allBus = String(bus[0]) + String(bus[1]) + String(bus[2]);
  radio.write(&allBus, 9);
  delay(3000);
}
