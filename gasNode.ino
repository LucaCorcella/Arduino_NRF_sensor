#include <nRF24L01.h>
#include <SPI.h>
#include <RF24.h>
#include "printf.h"

/* start customization */
RF24 radio(9, 10); //channel for Arduino Micro, for Arduino Uno you must change the channel in 7,8!!
/* end configuration */
const byte address[6] = "30000";

int data;
int gas_pin = A2;
int gasValue = 0;
int tempGas = -1;
bool ok;

void setup() {
  Serial.begin(115200);
  Serial.println("Gas test!");

  pinMode(gas_pin, INPUT);

  radio.begin();
  printf_begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(address);
  radio.stopListening();
  radio.printDetails();

}

void loop() {
  gasValue = analogRead(gas_pin);
  if (gasValue > 300) {
    data = 1;
  } else {
    data = 0;
  }

  if (tempGas != data) {
    Serial.print("Gas Val: ");
    Serial.println(data);

    ok = radio.write(&data, sizeof(data));
    if (!ok) {
      Serial.println("it failed to send");
    } else {
      tempGas = transmitter_gas.data;
      Serial.println("data sent");
    }
  }
  delay(1000);
}
