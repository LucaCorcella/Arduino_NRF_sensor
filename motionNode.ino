#include <nRF24L01.h>
#include <SPI.h>
#include <RF24.h>
#include "printf.h"

/* start customization */
RF24 radio(9, 10); //channel for Arduino Micro (9,10), for Arduino Uno you must change the channel in 7,8!!
/* end configuration */

const byte address[6] = "20000";

int data;
bool ok;

int motion_pin = 2;
int tempMotion = -1;

void setup() {
  Serial.begin(115200);
  Serial.println("Motion test!");

  pinMode(motion_pin, INPUT);

  radio.begin();
  printf_begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(address);
  radio.stopListening();
  radio.printDetails();
}

void loop() {
  data = digitalRead(motion_pin);
  if (tempMotion != data) {
    Serial.println("motion Val:");
    Serial.println(data);
    ok = radio.write(&data, sizeof(data));    
    if (!ok) {
      Serial.println("it failed to send");
    } else {
      tempMotion = data;
      Serial.println("data sent");
    }
  }
  delay(1000);
}
