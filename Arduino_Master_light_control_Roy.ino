#include <Ethernet.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

//Adaptation Platform constants
//const PROGMEM String environmentId = ""; //aaltestno10
//const PROGMEM String userName = "";
// auth0|5ab2b61acf2dd9296d671dff
const PROGMEM String environmentId = "auth0%7C5ab2b61acf2dd9296d671dffEnv"; //aaltestno10
const PROGMEM String userName = "auth0%7C5ab2b61acf2dd9296d671dff";
//const PROGMEM String environmentId = "auth0%7C5ab2b22ccf2dd9296d671dc9Env"; //aaltestno20
//const PROGMEM String userName = "auth0%7C5ab2b22ccf2dd9296d671dc9";
//const PROGMEM String environmentId = "auth0%7C59515ce63be5354b6a75effeEnv"; //Roy
//const PROGMEM String userName = "auth0%7C59515ce63be5354b6a75effe";
const PROGMEM String appName = "personAAL";

// roy test user
// auth0|59515ce63be5354b6a75effe
// auth0%7C59515ce63be5354b6a75effe

//Adaptation Engine - Context Manager IP
// byte server[] = { 192, 168, 2, 159 }; //
// const PROGMEM String serverHost = "192.168.2.159";
byte server[] = { 146, 48, 82, 160 }; //
const PROGMEM String serverHost = "146.48.82.160";

// IPAddress server(146, 48, 82, 160);       //giove.isti.cnr.it

//  Hue constants

// ROY: on-Tb1xLes9aYkBMeMhEYpKgbpEcFmxeeLX9rT7m
// https://www.developers.meethue.com/documentation/getting-started
// Philips Hue API: http://<bridge ip address>/api
// {"devicetype":"personaal_arduino"}
// http://<bridge ip address>/debug/clip.html

// const char hueHubIP[] = "192.168.143.248";  // Hue hub IP static
// const char hueUsername[] = "on-Tb1xLes9aYkBMeMhEYpKgbpEcFmxeeLX9rT7m";  // Hue username

const char hueHubIP[] = "192.168.1.50";  // Hue hub IP static - aaltestno10
const char hueUsername[] = "ZI14jjKhuWQEOn9pxqQyQQDrE5bW3UsLaYpDcxhh";  // Hue username
const int hueHubPort = 80;

//const char hueHubIP[] = "192.168.1.51";  // Hue hub IP static - aaltestno20
//const char hueUsername[] = "nJJXRw2yepMhYamMoHZplZXY6ucz2KqB66M-lNSN";  // Hue username
// Test person Bogerud: nJJXRw2yepMhYamMoHZplZXY6ucz2KqB66M-lNSN



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
  
  delay(3000);
  Serial.println(F("Here is the HUB..."));
  
  radio.begin();
  printf_begin();
  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS);
  
  Serial.print(F("radio.channel: "));
  Serial.println(radio.getChannel());
  
  radio.setPALevel(RF24_PA_LOW);
  
  radio.openReadingPipe(1, address1);
  radio.openReadingPipe(2, address2);
  radio.openReadingPipe(3, address3);
  radio.startListening();
  
  Serial.println(F("printdetails radio..."));
  radio.printDetails();  // hangs here if not printf_begin() has been called.
  
  Serial.println(F("Begin Ethernet setup..."));

  //Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  if (Ethernet.begin(mac) == 0) { //if (1) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    Ethernet.begin(mac, ip, dnServer, gateway, subnet);
    //Serial.println("Ethernet configured with static ip");
  } else {
    Serial.print(F("My IP address [DHCP]: "));
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
      Serial.print(Ethernet.localIP()[thisByte], DEC);
      Serial.print(".");
    }
    Serial.println();
  }

  delay(2000);
  
  Serial.print(F("userName: "));
  Serial.println(userName);
  Serial.print(F("appName: "));
  Serial.println(appName);
  Serial.println(F("Ready."));
}

void loop() {
  if ( radio.available(&pip) ) {
    Serial.print(F("pipe "));
    Serial.println(pip);

    if (pip == 1) {
      //DHT11
      float temperature[] = {0.00, 0.00};
      radio.read(&temperature, sizeof(temperature));
      Serial.print(F("temp "));
      Serial.println(temperature[0]);
      Serial.print(F("humidity "));
      Serial.println(temperature[1]);
      sendDataToCMViaPOST(temperature[0], temperature[1]);
    } else if (pip == 2) {
      //MOTION
      int motion = 0;
      radio.read(&motion, sizeof(motion));
      Serial.print(F("motion "));
      Serial.println(motion);
      sendDataToCMViaGET(pip, motion);
    } else if (pip == 3) {
      //gas
      int gas = 0;
      radio.read(&gas, sizeof(gas));
      Serial.print(F("gas "));
      Serial.println(gas);
      sendDataToCMViaGET(pip, gas);
    }
  } else {
    if (count == 100000) {
      
      Serial.println();
      Serial.println(F("=====>Checking for Hue update"));
      
      if (client.available()) {
        char c = client.read();
      }
      client.stop();
      /* Get new actions */
      Serial.println(F("Get new actions"));
     
      if (client.connect(server, 8880)) {
        Serial.print(F("Connected: polling for adaptation using: userName="));
        Serial.print(userName);
        Serial.print(F(" and appName="));
        Serial.println(appName);
        Serial.println();
        
        client.print("GET /NewAdaptationEngine/rest/pollForAdaptation/userName/");
        client.print(userName);
        client.print("/appName/");
        client.print(appName);
        client.println(" HTTP/1.0");
        client.print("Host: ");
        client.println(serverHost);
        client.println("");
        client.println("Connection: close");
        client.println();
      } else {
        Serial.println(F("connection failed"));
      }
      
      String readBuffer = "";
      String room = "";
      String command = "";
      boolean openFound = false;
      boolean closeFound = false;
      
      while ((client.connected() || client.available()) && !closeFound) {
        
        if (client.available()) {
          char c = client.read();
          Serial.print(c);
          
          // strip away start char
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
            // replace * with "
            if (c == '*') {
              readBuffer += "\"";
              continue;
            }
            
            if ((room == "" && c == ',') || (room != "" && c == '}')) {
              if (room == "") {
                room = readBuffer.substring(1, readBuffer.length()-1);
              } else {
                
                if (room != "\"no-actions\"") {
                  
                  command = readBuffer.substring(1, readBuffer.length()-1) +"}";
                  
                  //Serial.print(F("[Room: "));
                  //Serial.print(room);
                  //Serial.print(F("]"));
                  //Serial.print(", Command: ");
                  //Serial.println(readBuffer);
                  setHue(getLightIndex(room), command); // function change status light                  
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
      
      
      Serial.println();
      Serial.print(F("Room:"));
      Serial.println(room);
      Serial.print(F("readBuffer:"));
      Serial.println(readBuffer);
      Serial.print(F("command:"));
      Serial.println(command);
      
      count = 0;
    }
  }
  count = count + 1;
}



//change lights status
void setHue(int lightNum, String command) {
  
  Serial.print(F("setHue called. With lightNum:["));
  Serial.print(lightNum);
  Serial.print(F("], and command=["));
  Serial.print(command);
  Serial.println(F("]"));
  
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
    Serial.println(F("Not connected to context manager"));
  }

  String readBuffer = "";
  while (client.connected() || client.available()) {
    if (client.available()) {
      char c = client.read();
      readBuffer += c;
    }
  }
  Serial.print(F("CM: "));
  Serial.print(environmentId);
  Serial.print(F(" data: "));
  Serial.println(value);
  //Serial.print(F("CM res: "));
  //Serial.println(readBuffer);
}

void sendDataToCMViaPOST(float temp, float humidity) {
  String bodyData = "{\"temperature\":" + String(temp) + ", \"humidity\":" + String(humidity) + "}";
  Serial.print(F("bodyData: "));
  Serial.println(bodyData);
  
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
  Serial.print(F("CM: "));
  Serial.print(environmentId);
  Serial.print(F("data: "));
  Serial.print(bodyData);
  //Serial.print(F("res: "));
  //Serial.println(readBuffer);
}

void resetEthernet() {
  client.stop();
  delay(1000);
  Ethernet.begin(mac, ip);
  delay(2000);
}

int getLightIndex(String room) {  
  if (room == "LivingRoom")
    return 1;
  else if (room == "Kitchen")
    return 2;
  else if (room == "Entrance")
    return 3;
  else return 0;
}

