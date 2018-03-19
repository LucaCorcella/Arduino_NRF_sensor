#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
#include "DHT.h"

#define DHTPIN A0     // what analog pin we're connected to
#define DHTTYPE DHT11

/* start customization */
RF24 radio(9, 10); //channel for Arduino Micro, for Arduino Uno you must change the channel in 7,8!!
/* end configuration */


struct dataStruct {
  String nameSensor;
  int dataT;
  int dataH;
} transmitter_temperature;

unsigned char ADDRESS0[5]  =
{
  0xb1, 0x43, 0x88, 0x99, 0x45
};

DHT dht(DHTPIN, DHTTYPE);

int tmpTemperature = 0;
int tmpHumidity = 0;

void setup() {
  radio.begin();
  Serial.begin(115200);
  while (!Serial);
  dht.begin();
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
  transmitter_temperature.nameSensor = "temperatureNode";

  int h = dht.readHumidity();
  int t = dht.readTemperature();

  transmitter_temperature.dataT = t;
  transmitter_temperature.dataH = h;

  bool  ok;

  if (tmpTemperature != transmitter_temperature.dataT || tmpHumidity != transmitter_temperature.dataH ) {
    Serial.println("dh11 Val:");
    Serial.println(transmitter_temperature.dataT);
    Serial.println(transmitter_temperature.dataH);

    ok = radio.write(&transmitter_temperature, sizeof(transmitter_temperature));

    tmpTemperature = transmitter_temperature.dataT;
    tmpHumidity = transmitter_temperature.dataH;

    if (ok) {
      Serial.println("Pipe 1");
      Serial.println(transmitter_temperature.dataT);
      Serial.println(transmitter_temperature.dataH);
    } else {
      Serial.println("it failed to send");
    }
  }
  delay(1000);
}
