#include <Ethernet.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

//Adaptation Platform constants
const PROGMEM String environmentId = "auth0%7C5a3133847b89183611baac8eEnv"; //change it with your environment_id
const PROGMEM String userName = "auth0%7C5a3133847b89183611baac8e";
const PROGMEM String appName = "personAAL";
//Adaptation Engine - Context Manager IP
byte server[] = { 192, 168, 2, 159 }; //
const PROGMEM String serverHost = "192.168.2.159";

//  Hue constants
const char hueHubIP[] = "192.168.2.167";  // Hue hub IP static
const char hueUsername[] = "iAfN5IaNJ3SKF4b-piyUidQLci6lBQJDT2aDBbM-";  // Hue username
const int hueHubPort = 80;

//  Ethernet config
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x23, 0x36 }; //MAC address found on the back of your ethernet shield.
IPAddress ip(192, 168, 2, 106); // IP set static Ip address.
IPAddress dnServer(192, 168, 2, 1);// the dns
IPAddress gateway(192, 168, 2, 1);// the dns
IPAddress subnet(255, 255, 255, 0);// the router's gateway address.

/* end customization */
/**************************************************************************/
EthernetClient client;

long count = 0;

RF24 radio(7, 8); // CE, CSN
const byte address1[6] = "10000";
const byte address2[6] = "20000";
const byte address3[6] = "30000";
byte pip = 0;

void setup() {

  Serial.begin(9600);
  //Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  /*if (Ethernet.begin(mac) == 0) { */if (1) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip, dnServer, gateway, subnet);
    //Serial.println("Ethernet configured with static ip");
  } else {
    Serial.print("My IP address [DHCP]: ");
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
      Serial.print(Ethernet.localIP()[thisByte], DEC);
      Serial.print(".");
    }
    Serial.println();
  }

  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.openReadingPipe(1, address1);
  radio.openReadingPipe(2, address2);
  radio.openReadingPipe(3, address3);
  radio.startListening();
  radio.printDetails();


  delay(2000);
  Serial.println("Ready.");
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
  } else {
    if (count == 100000) {
      if (client.available()) {
        char c = client.read();
      }
      client.stop();
      /* Get new actions */
      if (client.connect(server, 8880)) {
        client.println("GET /NewAdaptationEngine/rest/pollForAdaptation/userName/" + userName + "/appName/" + appName + " HTTP/1.0");
        client.print("Host: ");
        client.println(serverHost);
        client.println("Connection: close");
        client.println();
      } else {
        Serial.println("connection failed");
      }
      String readBuffer = "";
      boolean openFound = false;
      boolean closeFound = false;
      String room = "";      
      while ((client.connected() || client.available()) && !closeFound) {
        if (client.available()) {
          char c = client.read();          
          if (c == '[' && !openFound) {
            openFound = true;
            continue;
          }
          /* Action Format:
              ["no-actions"]
              [
                Kitchen, {*on*: true,*xy*: [0.7006062,0.299301],*bri*:255, *transitiontime*:20}
                or
                {*on*:false,*transitiontime*20}
              ]
          */
          if (openFound) {
            if (c == '*') {
              readBuffer += "\"";
              continue;
            }
            if ((room == "" && c == ',') || (room != "" && c == '}')) {
              if (room == "") {
                room = readBuffer.substring(1, readBuffer.length()-1);
              } else {
                //cmd = readBuffer.substring(1) + "}";
                if (room != "\"no-actions\"") {
                  Serial.println("Room");
                  Serial.println(room);
                  Serial.println("Command");
                  Serial.println(readBuffer);
                  setHue(getLightIndex(room), readBuffer.substring(1) + "}"); // function change status light                  
                }
                closeFound = true;
              }
              readBuffer = "";
            } else {
              readBuffer += c;
            }
          }
        }
      }
      count = 0;
    }
  }
  count = count + 1;
}



//change lights status
void setHue(int lightNum, String command) {
  if (client.available()) {
    char c = client.read();
  }
  client.stop();
  if (client.connect(hueHubIP, hueHubPort)) {
    while (client.connected()) {
      client.print("PUT /api/");
      client.print(hueUsername);
      client.print("/lights/");
      client.print(lightNum);  // hueLight zero based, add 1
      client.println("/state HTTP/1.1");
      client.println("keep-alive");
      client.print("Host: ");
      client.println(hueHubIP);
      client.print("Content-Length: ");
      client.println(command.length());
      client.println("Content-Type: text/plain;charset=UTF-8");
      client.println();  // blank line before body
      client.println(command);  // Hue command
    }
    client.stop();
    //return true;  // command executed
  }
  //else
    //return false;  // command failed
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
    client.print("Host: ");
    client.println(serverHost);
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Not connected to context manager");
  }

  String readBuffer = "";
  while (client.connected() || client.available()) {
    if (client.available()) {
      char c = client.read();
      readBuffer += c;
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
    client.print("Host: ");
    client.println(serverHost);
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

void resetEthernet() {
  client.stop();
  delay(1000);
  Ethernet.begin(mac, ip);
  delay(2000);
}

int getLightIndex(String room) {  
  if (room == "Kitchen")
    return 1;
  else if (room == "LivingRoom")
    return 2;
  else if (room == "Entrance")
    return 3;
  else return 0;
}

