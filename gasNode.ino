#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

/* start customization */
RF24 radio(9, 10); //channel for Arduino Micro, for Arduino Uno you must change the channel in 7,8!!
/* end configuration */

struct dataStruct {
  String nameSensor;
  int data;
} transmitter_gas;


unsigned char ADDRESS0[5]  =
{
  0xb3, 0x43, 0x88, 0x99, 0x45
};

int gas_pin = A2;
int gasValue = 0;
int tempGas = -1;

void setup() {
  radio.begin();
  Serial.begin(115200);
  while (!Serial);
  pinMode(gas_pin, INPUT);
  printf_begin();
  radio.setRetries(15, 15);
  radio.enableDynamicPayloads();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(ADDRESS0);
  radio.openReadingPipe(0, ADDRESS0);
  radio.stopListening();
  radio.printDetails();

}

void loop() {
  transmitter_gas.nameSensor = "Gas";

  gasValue = analogRead(gas_pin);

  if (gasValue > 300) {
    transmitter_gas.data = 1;
  } else {
    transmitter_gas.data = 0;
  }

  bool ok;
  
  if(tempGas != transmitter_gas.data){
          Serial.println("Gas Val:");
          Serial.println(transmitter_gas.data);  
    
       ok = radio.write(&transmitter_gas, sizeof(transmitter_gas));
       tempGas = transmitter_gas.data;


        if(ok){
          Serial.println("Pipe 2");
          Serial.println(transmitter_gas.nameSensor);
          Serial.println(transmitter_gas.data); 
       }else {
          Serial.println("it failed to send");
       }
  }
      
  delay(1000);
}
