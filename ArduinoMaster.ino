#include <Ethernet.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

/**************************************************************************/
/* start customization */
String environmentId = "auth0%7C5a9e5eff864b1f3b20789fafEnv";//aaltestch10@gmail.com
//String environmentId = "auth0%7C5a9e5fb596cdb7109ddae110Env";//aaltestch20@gmail.com

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x23, 0x36 }; //MAC address found on the back of your ethernet shield.
/* These are the settings in case the router does not support DHCP configuration */
IPAddress ip(146, 48, 85, 69); // IP set static Ip address.
IPAddress dnServer(146, 48, 80, 3);// the dns
IPAddress gateway(146, 48, 80, 1);// the router's gateway address.
IPAddress subnet(255, 255, 248, 0);// the subnet.
/* end customization */
/**************************************************************************/
RF24 radio(7, 8); // CE, CSN
const byte address1[6] = "10000";
const byte address2[6] = "20000";
const byte address3[6] = "30000";
byte pip = 0;
byte pload_width_now;

EthernetClient client;
IPAddress server(146, 48, 82, 160);       //giove.isti.cnr.it
//IPAddress server(146, 48, 85, 134);         //taurus.isti.cnr.it

void setup() {
  Serial.begin(115200);
  Serial.println("Here is the HUB...");

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, address1);
  radio.openReadingPipe(2, address2);
  radio.openReadingPipe(3, address3);
  radio.startListening();
  radio.printDetails();

  if (1) { /*if (Ethernet.begin(mac) == 0) {*/
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip, dnServer, gateway, subnet);
    Serial.println("Ethernet configured with static ip");
  } else {
    Serial.print("My IP address [DHCP]: ");
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
      Serial.print(Ethernet.localIP()[thisByte], DEC);
      Serial.print(".");
    }
    Serial.println();
  }
  delay(1000);
}

void loop() {
  if ( radio.available(&pip) ) {
    Serial.print("pipe ");
    Serial.println(pip);

    if (pip == 1) {
      //DHT11
      float temperature[] = {0.00, 0.00};
      radio.read(&temperature, sizeof(temperature));
      Serial.print("temp ");
      Serial.println(temperature[0]);
      Serial.print("humidity ");
      Serial.println(temperature[1]);
      sendDataToCMViaPOST(temperature[0], temperature[1]);
    } else if (pip == 2) {
      //MOTION
      int motion = 0;
      radio.read(&motion, sizeof(motion));
      Serial.print("motion ");
      Serial.println(motion);
      sendDataToCMViaGET(pip, motion);
    } else if (pip == 3) {
      //gas
      int gas = 0;
      radio.read(&gas, sizeof(gas));
      Serial.print("gas ");
      Serial.println(gas);
      sendDataToCMViaGET(pip, gas);
    }
  }
}

void sendDataToCMViaGET(byte pipe, int value) {
  if (client.available()) {
    char c = client.read();    
  }
  client.stop();
  if (client.connect(server, 8880)) {
    Serial.println(F("Connected to CM"));
    if (pipe == 2) {
      if (value == 0) {
        client.println("GET /cm/rest/environment/" + environmentId + "/motion/false HTTP/1.1");
      } else {
        client.println("GET /cm/rest/environment/" + environmentId + "/motion/true HTTP/1.1");
      }
    } else if (pipe == 3) {      
      if (value == 0) {
        client.println("GET /cm/rest/environment/" + environmentId + "/gas-sensor/false HTTP/1.1");
      } else {
        client.println("GET /cm/rest/environment/" + environmentId + "/gas-sensor/true HTTP/1.1");
      }
    }
    client.println("Host: giove.isti.cnr.it");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Not connected to context manager");
  }

  String readBuffer = "";
  while (client.connected() || client.available()) {
    if (client.available()) {
      readBuffer += client.read();
    }
  }
  Serial.print("CM: ");
  Serial.print(environmentId);
  Serial.print(" data: ");
  Serial.println(value);  
  Serial.print("CM res: ");
  Serial.println(readBuffer);
}

void sendDataToCMViaPOST(float temp, float humidity) {
  String bodyData = "{\"temperature\":" + String(temp) + ", \"humidity\":" + String(humidity) + "}";  
  if (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
  client.stop();
  if (client.connect(server, 8880)) {
    Serial.println(F("Connected to CM"));
    client.println("POST /cm/rest/environment/" + environmentId + "/DHT11Sensors" + " HTTP/1.1");
    client.println("Host: giove.isti.cnr.it");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(bodyData.length());
    client.println("Connection: close");
    client.println();
    client.print(bodyData);
  }
  String readBuffer = "";
  while (client.connected() || client.available()) {
    if (client.available()) {
      readBuffer += client.read();
    }
  }
  Serial.print("CM: ");
  Serial.print(environmentId);
  Serial.print("data: ");
  Serial.print(bodyData);
  Serial.print("res: ");
  Serial.println(readBuffer);
}
