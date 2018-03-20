#include "DHT.h"
#include <nRF24L01.h>
#include <SPI.h>
#include <RF24.h>
#include "printf.h"

#define DHTPIN A0     // what analog pin we're connected to
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321

RF24 radio(9, 10);

const byte address[6] = "10000";
bool ok;

DHT dht(DHTPIN, DHTTYPE);
float tmpTemperature = 0;
float tmpHumidity = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("DHT11 test!");

  dht.begin();
  printf_begin();

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(address);
  radio.stopListening();
  radio.printDetails();

}

void loop() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  float temperature[] = {t, h}; //send temperatuere and humidity as array

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  if (tmpTemperature != t || tmpHumidity != h ) {
    Serial.println(temperature[0]);
    Serial.println(temperature[1]);
    ok = radio.write(&temperature, sizeof(temperature));
    if (!ok) {
      Serial.println("Failed to send!");
    } else {
      tmpTemperature = t;
      tmpHumidity = h;
    }
  }
  delay(5000);

}
