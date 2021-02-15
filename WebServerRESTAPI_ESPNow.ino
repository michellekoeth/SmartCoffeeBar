
// This code is intended for an ESP-32 Board
#include <esp_now.h>
#include <WiFi.h>
//#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <HTTPClient.h>

// Ported from Arduino
#include <AccelStepper.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define dirPin 19 // labelled as RX2 - pod sled stepper
#define stepPin 23 // labelled as TX2 - pod sled stepper
#define motorInterfaceType 1
#define stagedirPin 16 // labelled as RX2 - cup moving linear stage
#define stagestepPin 17 // labelled as TX2 - cup moving linear stage
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
// steps on coffee cup linear stage from Zero point (where you put cup) to front of coffee machine
const int coffeeMachineStepNo = 3600;
// Step positions on long linear stage that runs to multiple drink stations
const int rightmostBinPos = 6500;
const int binTwoPos = 8500;
const int binThreePos = 10000;
const int binFourPos = 11300;
const int binFivePos = 12250;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

static const uint8_t image_data[1024] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc3, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe1, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x02, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x07, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x07, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

Servo podSledServo;

// Create a new instance of the AccelStepper class for the pod sled stepper
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
AccelStepper stagestepper = AccelStepper(motorInterfaceType, stagestepPin, stagedirPin);



// output for driving the pod sled servo motor
const int podSledServoPin = 18;

// ESP NOW code - These mac addresses must be those of the 8266 boards to communicate with
uint8_t broadcastAddressCoffeeProc[] = {0xF4, 0xCF, 0xA2, 0xF7, 0x62, 0x7A};
//uint8_t broadcastAddressPowderDispenserProc[] = {0xF4, 0xCF, 0xA2, 0xF7, 0x62, 0x7A};

// ESP NOW Message Format
typedef struct struct_message {
  char msgtype[32];
  int typecategory;
  bool boolval;
} struct_message;

// Create a struct_message called myData
struct_message myData;
struct_message sendDataPacket;

// Variable to store if sending data was successful
String success;

WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP32 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from Drink Server on the ESP32!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

// ESP NOW DATA listeners/senders
// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  Serial.println("Incoming Data Received");
  memcpy(&myData, incomingData, sizeof(myData));
  // what type of data has been received?
  Serial.println("=== Data ===");
  Serial.print("Mac address: ");
  for (int i = 0; i < 6; i++) {
       Serial.print("0x");
       Serial.print(mac[i], HEX);
      Serial.print(":");
  }
    
  Serial.print("\n\nMsgtype: ");
  Serial.println(myData.msgtype);
  Serial.print("\ntypecategory: ");
  Serial.println(myData.typecategory);
  Serial.println();
  String msgtype(myData.msgtype);
  /*if (msgtype == "STATUS") {
      // stuff to do when it's a status message
      if (myData.boolval) {
        Serial.println("SUCCESS status message received from ESP-8266");
      }
      else {
        Serial.println("FAILURE status message received from ESP-8266");
      }
  }
  else if (strstr("COMMAND",myData.msgtype)) {
      // stuff to do when the secondary device needs this primary server to do something
  }
*/
}

void setup(void) {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  podSledServo.attach(podSledServoPin);
  podSledServo.write(20); 
 
  // Set the maximum speed and acceleration for pod sled stepper
  stepper.setMaxSpeed(5000);
  stepper.setAcceleration(2500);
  stepper.setCurrentPosition(0);
  stagestepper.setMaxSpeed(1000);
  stagestepper.setAcceleration(500);
  stagestepper.setCurrentPosition(0);
  digitalWrite(led, 0);
  while (!Serial) continue;
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  // Once ESPNow is successfully Init
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add/register peer coffee processor secondary device   
  memcpy(peerInfo.peer_addr, broadcastAddressCoffeeProc, 6);   
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add coffee processor device");
    return;
  }  
  /*
  // Add/register peer powder dispenser  secondary device
  memcpy(peerInfo.peer_addr, broadcastAddressPowderDispenserProc, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add powder dispenser device");
    return;
  }
  */

   // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  delay(1000);
  // Clear the buffer.
  // Draw bitmap on the screen
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("Coffee"));
  display.println(F("System"));
  //display.setCursor(0, 27);
  display.println(F("READY"));
  display.display();      
  //delay(5000);
  //display.clearDisplay();
  //display.drawBitmap(0, 0, image_data, 128, 64, 1);
  //display.display();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Soft AP Mac address: ");
  Serial.println(WiFi.softAPmacAddress());
  display.setTextSize(1); // Draw 1X-scale text
  display.println(WiFi.localIP());
  display.display();
  
  // Set server routing RESTful
  restServerRouting();
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  //timeClient.begin();
}

// Define routing
void restServerRouting() {
  server.on("/", handleRoot);
  //server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.on("/openPodChamber", openPodChamber);
  server.on("/closePodChamber", closePodChamber);
 // server.on("/openPodChamberMachineIsOn", openPodChamberMachineIsOn);
 // server.on("/turnOffMachine", turnOffMachine);
 // server.on("/linearActuatorUp", linearActuatorUp);
 // server.on("/linearActuatorDown", linearActuatorDown);
 // server.on("/linearActuatorStop", linearActuatorStop);
  server.on("/podSledJiggle", podSledJiggle);
 // server.on("/turnOnMachine", turnOnMachine);
  server.on("/pressTopButton", pressTopButton);
  server.on("/closePodChamberPressTopButton", closePodChamberPressTopButton);
  server.on("/moveStage", moveStage);
  //server.on("/podSledDown", podSledDown);
  server.on("/makeCoffee", makeCoffee);
  server.on("/moveSledStepperIntoPodChamber", moveSledStepperIntoPodChamber);
  server.on("/moveSledStepperOutOfPodChamber", moveSledStepperOutOfPodChamber);
  server.on("/resetSledServo", resetSledServo);
  //server.on("/readLEDColor", readLEDColor);
  //server.on("/dispenseBin", dispenseBin);
}

void loop(void) {
  unsigned long currentMillis = millis();
  unsigned long previousMillis = 0;
// if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=2000)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
  server.handleClient();
  // ESP32 is using ArduinoJson version 6 
  //monitorLED(1);
  /*display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Coffee"));
  display.println(F("System"));
  display.println(F("READY"));
  display.println(WiFi.localIP());
  display.display();    
  delay(2000);
  display.clearDisplay();
  display.drawBitmap(0, 0, image_data, 128, 64, 1);
  display.display();
  delay(2000); */
}

/*****
 * Route functions will havw to be refactored to delegate certain functions now to be 
 * performed by the secondary ESP-8266 boards via ESP-NOW sent messages
 * 
 * The send message format is: - replace broadcast Address with the mac address of the device
 * to send to. Populate the values of the sendData struct with the request
 * 
 for ex:
 strcpy(sendData.msgtype, "COMMAND"); // send a command to the 8266
  sendData.msgtype = "COMMAND";
  sendData.typecategory = 1;
  sendData.boolval = true;
  

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendData, sizeof(sendData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
 * 
 */


void makeCoffee() {
  int requestedDelay = server.arg("delay").toInt();
  if(requestedDelay == 0) requestedDelay = 120000;
  Serial.print("Delay to return to stage set to: ");
  Serial.println(requestedDelay);
  // Draw bitmap on the screen
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Turning on");
  display.display();      
  pressTopButton();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Moving\nStage");
  display.display();      
  moveStageAmt(coffeeMachineStepNo);
   display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Open Chamber");
  display.display();  
  //openPodChamberMachineIsOn();
  openPodChamber();
   display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Move Pod\nSled");
  display.display();  
  moveSledStepperIntoPodChamber();
   display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Jiggle\nPod");
  display.display(); 
  podSledJiggle();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Move Sled\nBack");
  display.display(); 
  moveSledStepperOutOfPodChamber();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Reset\nServo");
  display.display(); 
  resetSledServo();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Close\nChamber");
  display.display();
  closePodChamber();
  //Now also need to wait 3 seconds so that it is closed for sure before pressing top button
  delay(2000);
  pressTopButton();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Brew\nCoffee");
  display.display();
  
  //timeClient.update();
  //unsigned long startTime =  timeClient.getEpochTime(); // start epoch time
  //unsigned long timeelapsed = 0;
  // Monitor the pod lamp for when it stops blinking - then you know coffee is done
  // Function takes in a monitoring mode as it's variable, 1 is for coffee in progress to coffee done monitoring
  /* Need to refactor this code with calls to the Coffee 8266 client since it's handling the color sensing now
  String returnstr = "";
  while (true) {
    returnstr = monitorLED(1);
    if (returnstr == "solidgreen") {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Coffee is\nbrewed");
      display.display();
      delay(2000); // delay 2 seconds for drip delay
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Finished");
      display.display();
      break;
    }
    else if (returnstr == "chasegreen") {
      //display.clearDisplay();
      //display.setCursor(0, 0);
      //timeelapsed = timeClient.getEpochTime() - startTime;
      //String timeelapsedstr = String(timeelapsed, DEC);
      //display.println("Brewing in\nprogress\n" + timeelapsedstr + " sec");
      //display.display();
    }
    else if (returnstr == "blinkred") { // This should happen if there is an error in the pod loading - reset station
      // Blink error on display five times
      for (int j=0; j<4; j++) {
        display.clearDisplay();
        delay(500);
        display.setCursor(0, 0);
        display.println("ERROR");
        delay(500);
        display.display();
      }
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Returning\nTo\nStart");
      delay(500);
      display.display();
      break;
    }
  }
  */
  delay(requestedDelay); // for now use a set delay of 3 minutes for coffee to brew
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Finished");
  delay(500);
  display.display();
  moveStageAmt(0);
}


void moveStageAmt(int posNum) {
   // Move that number of steps
  stagestepper.moveTo(posNum);
  stagestepper.runToPosition();
  delay(100);
  server.send(200, "text/plain", "Moved Stage");
}

/*
String monitorLED(int modenum) {
  uint16_t r, g, b, c, colorTemp, lux;
  //uint16_t vals[5][43] = {{}};
  int luxzerocnt = 0;
  int redcnt = 0;
  int greencnt = 0;
  String statusstr = "";
  for(int i=0;i<43;i++) { //Resolution set 24ms integration time, so 43 samples is one seconds worth
      tcs.getRawData(&r, &g, &b, &c);
      colorTemp = tcs.calculateColorTemperature(r, g, b);
      lux = tcs.calculateLux(r, g, b);
      if (lux < 10) luxzerocnt++;
      if (r > 2000 ) redcnt++;
      if ((g > 100) && (r < 500)) greencnt++;
      // TODO: will have to figure out what orange is
      // Signature for no pod when top button is clicked:
      // Lux will be greater than 5, red will be greater than 5 
  } 
  if ((luxzerocnt > 5) && (redcnt > 5)) statusstr = "blinkred";
  else if (greencnt > 35) statusstr = "solidgreen";
  else if ((luxzerocnt < 2) && (greencnt > 3 )) statusstr = "chasegreen"; //when coffee is brewing 
  switch(modenum) {
    case 1: { // here we are monitoring the coffee generation process
      display.clearDisplay();
      display.setTextSize(2); // Draw 2X-scale text
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      String luxcntstr = String(luxzerocnt, DEC);
      String redcntstr = String(redcnt, DEC);
      String greencntstr = String(greencnt, DEC);
      String displaystr = "Lux0: " + luxcntstr + "\nRed: " + redcntstr+ "\nGreen: " + greencntstr;
      display.println(displaystr);
      display.display();
      break;
    }  
    case 2:
      // TODO
      Serial.println("No code here yet");
      break;
  }
  return statusstr;
}*/
/*
void readLEDColor() {
  uint16_t r, g, b, c, colorTemp, lux;
  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  lux = tcs.calculateLux(r, g, b);
  String red1 = String(r, DEC);
  String green1 = String(g, DEC);
  String blue1 = String(b, DEC);
  String luxstr = String(lux, DEC);
  String colorValStr = "R: " + red1 + "\nG: " + green1 + "\nB: " + blue1 + "\nLux: " + luxstr;
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(colorValStr);
  display.display();      
}

void dispenseBin() {
  String binNo = server.arg("bin");
  // Call out to bin controller
  int stepno2 = 0;
  if(binNo == "1") { 
    stepno2 = rightmostBinPos;
    String stepnoAmt = String(stepno2);
    Serial.println("Moving to position: " + stepnoAmt);
  }
  else if(binNo == "2") {
    stepno2 = binTwoPos;
    String stepnoAmt = String(stepno2);
    Serial.println("Moving to position: " + stepnoAmt);
  }
  else if(binNo == "3") {
    stepno2 = binThreePos;
  }
  else if(binNo == "4") {
    stepno2 = binFourPos;
  }
  else if(binNo == "5") {
    stepno2 = binFivePos;
  }
  else {
    return;
  }
  Serial.println("Moving to bin: " + binNo);
  stagestepper.moveTo(stepno2);
  stagestepper.runToPosition();
  //delay(100);
  WiFiClient client2;
  HTTPClient http;
  http.begin(client2, "http://192.168.101.183/dispenseBin?bin="+ binNo);
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
    http.end(); // end the http connection here before returning status locally
    Serial.println("Successful http.end");
    //delay(500);
    server.send(200, "text/plain", "Successful bin dispensing");
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    http.end();
    delay(1000);
    server.send(400, "text/plain", "Error");
  }
}*/

void moveStage() {
  //args of URL will be dir and stepno
  int stepno = server.arg("steppos").toInt();
  // set the direction
  // Move that number of steps
  stagestepper.moveTo(stepno);
  stagestepper.runToPosition();
  delay(100);
  server.send(200, "text/plain", "Moved Stage");
}

void moveSledStepperIntoPodChamber() {
  stepper.moveTo(80000);
  stepper.runToPosition();
  delay(100);
  server.send(200, "text/plain", "Command to move pod sled into pod chamber ");
}

void moveSledStepperOutOfPodChamber() {
  stepper.moveTo(0);
  stepper.runToPosition();
  delay(100);
   server.send(200, "text/plain", "Command to move pod sled outof pod chamber ");
}


void resetSledServo() {
  podSledServo.write(20);
  server.send(200, "text/json", "{\"response\": \"Pod Sled Servo Reset \"}");
}

void podSledJiggle() {
  //podSledServo.write(15);
  delay(1000);
  podSledServo.write(30);
  delay(1000);
  podSledServo.write(20);
  delay(1000);
  podSledServo.write(35);
  delay(1000);
  podSledServo.write(30);
  delay(1000);
  //podSledServo.write(20);
  server.send(200, "text/json", "{\"response\": \"Pod Sled Jiggled \"}");
}

void podSledDown() {
  podSledServo.write(10);
  delay(1500);
  server.send(200, "text/json", "{\"response\": \"Pod Sled  Lowered\"}");
}

void pressTopButton() {
  // Send a command packet to the ESP-8266 controlling the coffee maker to press the top button
  strcpy(sendDataPacket.msgtype, "COMMAND"); // send a command to the 8266
  sendDataPacket.typecategory = 1; // type 1 is for pressTopButton
  sendDataPacket.boolval = true;
  

  esp_err_t result = esp_now_send(broadcastAddressCoffeeProc, (uint8_t *) &sendDataPacket, sizeof(sendDataPacket));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
      server.send(200, "text/json", "{\"response\": \"Top Button Pressed\"}");
    }
    else {
      Serial.println("Error sending the data");
      server.send(200, "text/json", "{\"response\": \"Error sending message to Coffee 8266 to press Top Button\"}");
    }
 
  
}

void closePodChamberPressTopButton() {
  // Send a command packet to the ESP-8266 controlling the coffee maker to press the top button
  strcpy(sendDataPacket.msgtype, "COMMAND"); // send a command to the 8266
  sendDataPacket.typecategory = 4; // type 4 is to issue both a close pod chamber and press top button command
  sendDataPacket.boolval = true;
  

  esp_err_t result = esp_now_send(broadcastAddressCoffeeProc, (uint8_t *) &sendDataPacket, sizeof(sendDataPacket));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
      server.send(200, "text/json", "{\"response\": \"Pod Chamber closed AND Top Button Pressed\"}");
    }
    else {
      Serial.println("Error sending the data");
      server.send(200, "text/json", "{\"response\": \"Error sending message to Coffee 8266 to press Top Button\"}");
    }
 
  
}


void openPodChamber() {
  // Do two up lever pushes which will raise the pod chamber no matter if the machine is
  // on or not
  // Simulate turning on & opening pod chamber
  strcpy(sendDataPacket.msgtype, "COMMAND"); // send a command to the 8266
  sendDataPacket.typecategory = 2; // type 2 is for openPodChamber
  sendDataPacket.boolval = true;
  

  esp_err_t result = esp_now_send(broadcastAddressCoffeeProc, (uint8_t *) &sendDataPacket, sizeof(sendDataPacket));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  server.send(200, "text/json", "{\"response\": \"Pod Chamber Opened with no assumptions about machine status\"}");
}
 
 /*
void turnOnMachine() {
  // Do two up lever pushes which will raise the pod chamber no matter if the machine is
  // on or not
  // Simulate turning on & opening pod chamber
  digitalWrite(upButtonRelay, LOW);
  delay(500);
  digitalWrite(upButtonRelay, HIGH);
  delay(500);
  server.send(200, "text/json", "{\"response\": \"Coffee Machine is turned On\"}");
}*/
/*
void openPodChamberMachineIsOn() {
  // Do two up lever pushes which will raise the pod chamber no matter if the machine is
  // on or not
  // Simulate just opening pod chamber - just one up button press
  digitalWrite(upButtonRelay, LOW);
  delay(500);
  digitalWrite(upButtonRelay, HIGH);
  server.send(200, "text/json", "{\"response\": \"Pod Chamber Opened assuming machine on\"}");
}*/

void closePodChamber() {
  // Do one lever push down which will close the pod chamber 
  // Simulate turning on & opening pod chamber
  strcpy(sendDataPacket.msgtype, "COMMAND"); // send a command to the 8266
  sendDataPacket.typecategory = 3; // type 3 is for closePodChamber
  sendDataPacket.boolval = true;
  esp_err_t result = esp_now_send(broadcastAddressCoffeeProc, (uint8_t *) &sendDataPacket, sizeof(sendDataPacket));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  server.send(200, "text/json", "{\"response\": \"Pod Chamber Closed\"}");
}
/*
void turnOffMachine() {
  // Do one lever push down which will close the pod chamber 
  // Simulate turning on & opening pod chamber
  digitalWrite(downButtonRelay, LOW);
  delay(3500);
  digitalWrite(downButtonRelay, HIGH);
  delay(1000);
  server.send(200, "text/json", "{\"response\": \"Machine turned off\"}");
}*/

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}
