/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp8266-nodemcu-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
//#include <Adafruit_TCS34725.h>

//#define SCREEN_WIDTH 128  // OLED display width, in pixels
//#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define LED 2

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_1X);
// Pins D1 and D2 are used by the adafruit color sensor

// REPLACE WITH RECEIVER MAC Address // ESP32 address
uint8_t broadcastAddress[] = {0xAC, 0x67, 0xB2, 0x38, 0x62, 0x39};
//trying this based on https://rntlab.com/question/esp8266-to-esp32-wifi-esp-now-problem/
//uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const int upButtonRelay = 14; // This is D5 input GPIO14
const int downButtonRelay = 12; // This is D6 input GPIO12
const int topButtonRelay = 13; // This is D7 input GPIO13

// Structure example to send data
// Must match the receiver structure
// ESP NOW Message Format
typedef struct struct_message {
  char msgtype[32];
  int typecategory;
  bool boolval;
} struct_message;

bool msgToSendFlag = false;

struct_message myData;
struct_message sendDataPacket;

unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;  // send readings timer


constexpr char WIFI_SSID[] = "Kenny";


// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.println("Inside On Data sent");
  //char macStr[18];
  //Serial.print("Packet to: ");
  //snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
   //        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //Serial.print(macStr);
  Serial.print(" send status:\t");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
    Serial.println(sendStatus);
  }
}

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  int msgtypeint = 0;
  Serial.println("RECEIVED DATA!!!");
  memcpy(&myData, incomingData, sizeof(myData));
  int commandType = myData.typecategory;
  String msgtype(myData.msgtype);
  Serial.println("Data received is of length (in Bytes): ");
  Serial.println(len);
  Serial.println("Message type: " + msgtype);
  Serial.println(myData.typecategory);
  // what type of data has been received?
  // Switch statements operate on intergers only, so map the strings over to ints
  if(msgtype == "STATUS") msgtypeint = 1;
  else if (msgtype == "COMMAND") { msgtypeint = 2; Serial.println("Received a command");}
  else if (msgtype == "FOO") msgtypeint = 3;
  switch(msgtypeint) {
    case 1: // STATUS request from ESP-32
      // stuff to do when it's a status message from ESP-32
      break;
    case 3: 
      // stuff to do when from ESP-32
      Serial.println("Message type 3 received");
      break;
    case 2: // Command from ESP-32
      // stuff to do when the ESP-32 primary is commanding this proc to do something
      //map command type for switch processing
      Serial.println("Got a COMMAND from ESP-32");
      switch(commandType) {
        case 1: {
          Serial.println("Got to Case 1 to pressTopButton");
          if(pressTopButton()) {
            strcpy(sendDataPacket.msgtype, "STATUS");
            sendDataPacket.typecategory = 1;
            sendDataPacket.boolval = true;
            esp_now_send(broadcastAddress, (uint8_t *) &sendDataPacket, sizeof(sendDataPacket));
          }
          break;
        }
        case 2: {
          if(openPodChamberMachineIsOn()) {
            // will happen if true is returned from openPodChamber Function
            sendDataPacket.typecategory = 2;
            strcpy(sendDataPacket.msgtype, "STATUS");
            sendDataPacket.boolval = true;
            esp_now_send(broadcastAddress, (uint8_t *) &sendDataPacket, sizeof(sendDataPacket));
          }
          else {
            Serial.println("ERROR with openPodChamber Function.. Exiting");
          }
          break;
        }
        case 3: {
          Serial.println("Command to close pod chamber");
          if(closePodChamber()) {
            // will happen if true is returned from closePodChamber Function
            sendDataPacket.typecategory = 3;
            strcpy(sendDataPacket.msgtype, "STATUS");
            sendDataPacket.boolval = true;
            // Send message via ESP-NOW
            esp_now_send(broadcastAddress, (uint8_t *) &sendDataPacket, sizeof(sendDataPacket));
          }
          break;
        }
        case 4: {
          Serial.println("Command to close pod chamber and press top button");
          if(closePodChamberAndPressTopButton()) {
            // will happen if true is returned from closePodChamber Function
            sendDataPacket.typecategory = 4;
            strcpy(sendDataPacket.msgtype, "STATUS");
            sendDataPacket.boolval = true;
            // Send message via ESP-NOW
            esp_now_send(broadcastAddress, (uint8_t *) &sendDataPacket, sizeof(sendDataPacket));
          }
          break;
        }
      }
      break;
  } 
  
}
 
void setup() {
  //pinMode(LED, OUTPUT); // Initialize the LED pin as an output
  pinMode(upButtonRelay, OUTPUT);
  pinMode(downButtonRelay, OUTPUT);
  pinMode(topButtonRelay, OUTPUT);
  digitalWrite(upButtonRelay, HIGH);
  digitalWrite(downButtonRelay, HIGH);
  digitalWrite(topButtonRelay, HIGH);
  // Init Serial Monitor
  Serial.begin(115200);
  /*if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  */
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

 
  

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
   
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
   // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
 
  
 
}
 
void loop() {
    digitalWrite(LED, LOW); // Turn the LED on (Note that LOW is the voltage level)
}

bool pressTopButton() {
  // Simulate pressing the top button by triggering the relay quickly
  Serial.println("Inside the pressTopButton function now");
  digitalWrite(topButtonRelay, LOW);
  delayMicroseconds(1000000);
  digitalWrite(topButtonRelay, HIGH);
  //Serial.println("About to send packet");
  // Send message via ESP-NOW
  Serial.println("Leaving the pressTopButton function");
  return true;
}

bool openPodChamberMachineIsOn() {
  // Do two up lever pushes which will raise the pod chamber no matter if the machine is
  // on or not
  // Simulate just opening pod chamber - just one up button press
  digitalWrite(upButtonRelay, LOW);
  //a bug in the 8266 compiler requires use of delayMicroseconds for any delay when a routine is run in an interrupt
  // see https://circuits4you.com/2017/12/19/esp8266-fatal-exception-wdt-reset/
  delayMicroseconds(1000000);
  digitalWrite(upButtonRelay, HIGH);
  return true;
}

bool openPodChamber() {
  // Do two up lever pushes which will raise the pod chamber no matter if the machine is
  // on or not
  // Simulate turning on & opening pod chamber
  digitalWrite(upButtonRelay, LOW);
  delayMicroseconds(1000000);
  digitalWrite(upButtonRelay, HIGH);
  //delayMicroseconds(1000000);
  //digitalWrite(upButtonRelay, LOW);
  //delayMicroseconds(1000000);
  //digitalWrite(upButtonRelay, HIGH);
  return true;
}

bool closePodChamber() {
  // Do one lever push down which will close the pod chamber 
  // Simulate turning on & opening pod chamber
  digitalWrite(downButtonRelay, LOW);
  delayMicroseconds(1000000);
  digitalWrite(downButtonRelay, HIGH);
  //delayMicroseconds(1000000);
  return true;
}

bool closePodChamberAndPressTopButton() {
  // Do one lever push down which will close the pod chamber 
  // Simulate turning on & opening pod chamber
  Serial.println("Inside the closePodChamberAndPressTopButton function now");
  digitalWrite(downButtonRelay, LOW);
  delayMicroseconds(500000);
  digitalWrite(downButtonRelay, HIGH);
  delayMicroseconds(500000);
  digitalWrite(topButtonRelay, LOW);
  delayMicroseconds(500000);
  digitalWrite(topButtonRelay, HIGH);
  //delayMicroseconds(2000000); // add 1 sec delay before sending response msg
  return true;
}

bool turnOffMachine() {
  // Do one lever push down which will close the pod chamber 
  // Simulate turning on & opening pod chamber
  digitalWrite(downButtonRelay, LOW);
  delayMicroseconds(3500000);
  digitalWrite(downButtonRelay, HIGH);
  delayMicroseconds(3500000);
  return true;
}
