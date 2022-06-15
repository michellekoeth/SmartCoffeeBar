
// This code is intended for an ESP-32 Board
#include <esp_wifi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <HTTPClient.h>
#define motorInterfaceType 1
#include <AccelStepper.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <qrcode.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128 // for SSD1351
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define DEBOUNCE_TIME 1000
volatile uint32_t DebounceTimer = 0;
// steps on coffee cup linear stage from Zero point (where you put cup) to front of coffee machine
const int coffeeMachineStepNo = 3600;
// Step positions on long linear stage that runs to multiple drink stations
const int rightmostBinPos = 6500;
const int binTwoPos = 8500;
const int binThreePos = 10000;
const int binFourPos = 11300;
const int binFivePos = 12250;
uint32_t button_count = 0;
const char* ssid = "Coffeemaker";
const char* password = "bean1234";

bool machineOff = true; // default to machine being off
bool machineStatusFresh = false; // default as false meaning the machine status is not fresh

const int upButtonRelay = 14; // This is D14 input GPIO14
const int downButtonRelay = 15; // This is D15 input GPIO15 // boot fails if connected to GPIO12 so dont use that one
const int topButtonRelay = 13; // This is D13 input GPIO13
const int podSledServoPin = 27; // D27 which is GPIO 287
const int podHopperServoPin = 33; // D33 which is GPIO 33
const int gate2ServoPin = 19; //D19, GPIO19
const int gateServoPin = 32; // D32 which is GPIO 32
const int dirPin = 26; // D26
const int stepPin = 25; // D25
//const int INTERRUPT_PIN = 4; // Big yellow coffee making button
// For the SSD1351 display:
#define SCLK_PIN 18
#define MOSI_PIN 23
#define DC_PIN   16
#define CS_PIN   17
#define RST_PIN  5 // outputs PWM signal at boot
// Color definitions
#define TFT_BLACK       0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define TFT_WHITE       0xFFFF
const int QRcode_Version = 11;   //  set the version (range 1->40) - MMK note I think this sets the size
const int QRcode_ECC = 0;       //  set the Error Correction level (range 0-3) or symbolic (ECC_LOW, ECC_MEDIUM, ECC_QUARTILE and ECC_HIGH)
#define _QR_doubleSize    //
#define Lcd_X  128
#define Lcd_Y  128
int stepAmtint = 0;
bool movePodSledStepperFlag = false;
bool makeCoffeeFlag = false;

Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);  
QRCode qrcode;
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

Servo podSledServo;


AsyncWebServer server(80);

void setup(void) {
  pinMode(upButtonRelay, OUTPUT);
  pinMode(downButtonRelay, OUTPUT);
  pinMode(topButtonRelay, OUTPUT);
  digitalWrite(upButtonRelay, HIGH);
  digitalWrite(downButtonRelay, HIGH);
  digitalWrite(topButtonRelay, HIGH); // uncommented to fix top button issue
  //pinMode(INTERRUPT_PIN, INPUT_PULLDOWN);
  //attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), setMakeCoffeeFlag, RISING); // trigger when button pressed, but not when released.
  Serial.begin(115200);
  stepper.setMaxSpeed(5000);
  stepper.setAcceleration(2500);
  stepper.setCurrentPosition(0);
  podSledServo.attach(podSledServoPin);
  podSledServo.write(10); 
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 32);
  tft.setTextSize(2);  //(30 pixels tall I think)

  while (!Serial) continue;
  WiFi.mode(WIFI_STA); // might need to be just WIFI_STA
  WiFi.begin(ssid, password);
  Serial.println("");

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
  uint8_t qrcodeData[qrcode_getBufferSize(QRcode_Version)];
  String WiFiAddy = "http://"+WiFi.localIP().toString();
  const char * wifichars = WiFiAddy.c_str();
  qrcode_initText(&qrcode, qrcodeData, QRcode_Version, QRcode_ECC, wifichars);    // 51 bytes.
  tft.fillScreen(TFT_WHITE);
  uint8_t x0 = 2;
  uint8_t y0 = 2;
  //display QRcode
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {

      if (qrcode_getModule(&qrcode, x, y) == 0) {     //change to == 1 to make QR code with black background
          //tft.drawPixel(x0 + x, y0 + y, TFT_BLACK); // - for smaller size
          tft.drawPixel(x0 + 2 * x,     y0 + 2 * y, TFT_BLACK);
          tft.drawPixel(x0 + 2 * x + 1, y0 + 2 * y, TFT_BLACK);
          tft.drawPixel(x0 + 2 * x,     y0 + 2 * y + 1, TFT_BLACK);
          tft.drawPixel(x0 + 2 * x + 1, y0 + 2 * y + 1, TFT_BLACK);
      }
    }
  }  
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Soft AP Mac address: ");
  Serial.println(WiFi.softAPmacAddress());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String headerpage = "<html><head><title>Coffee System 1.0</title><style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style></head><body><h1>Welcome to the MyCup Ultra Coffeematic 2000</h1>";
    String ipaddystr = WiFi.localIP().toString();
    String bodyStr = headerpage + "<B><a href='http://" + ipaddystr + "/pressUpButton'>Press Up Button</a></B><br>";
    bodyStr = bodyStr + "<B><a href='http://" + ipaddystr + "/pressDownButton'>Press Down Button</a></B><br>";
    bodyStr = bodyStr + "<B><a href='http://" + ipaddystr + "/pressTopButton'>Press Top Button</a></B><br>";
    bodyStr = bodyStr + "<B><a href='http://" + ipaddystr + "/moveSledStepper?stepno=40000'>Press Sled Stepper Into Coffee Machine</a></B><br>";
    bodyStr = bodyStr + "<B><a href='http://" + ipaddystr + "/moveSledStepper?stepno=0'>Press Sled Stepper Back To Origin</a></B><br>";
    bodyStr = bodyStr + "<B><a href='http://" + ipaddystr + "/podSledJiggle'>Jiggle Pod Sled</a></B><br>";
    bodyStr = bodyStr + "<B><a href='http://" + ipaddystr + "/podSledReset'>Reset Pod Sled Servo</a></B><br>";
    bodyStr = bodyStr + "<br><br><h1><a href='http://" + ipaddystr + "/makeCoffee'>Run all routines and make coffee</a></h1></B><br>";
    String returnStr = bodyStr + "</body></html>";
    request->send_P(200, "text/html", returnStr.c_str());
    
  });


  // Route to do something
  server.on("/podSledMoveUp", HTTP_GET, [](AsyncWebServerRequest *request){
    //podSledServo.write(10);
    request->send_P(200, "text/html", "Pod Sled Moved Up");
  });



  // Route to do something
  server.on("/moveCupStage", HTTP_GET, [](AsyncWebServerRequest *request){
    int stepamt = 0;
    String stepamtstr;
    if (request->hasParam("stepno")) {
      stepamtstr = request->getParam("stepno")->value();
      stepamt = stepamtstr.toInt();
    }
    //moveCupStage(stepamt);
    request->send_P(200, "text/html", "moved cup stage");
  });
  
   server.on("/podSledMoveDown", HTTP_GET, [](AsyncWebServerRequest *request){
      podSledServo.write(30);
    request->send_P(200, "text/html", "Pod Sled Moved Down");
  });

  server.on("/podSledJiggle", HTTP_GET, [](AsyncWebServerRequest *request){
    podSledJiggle();
    String ipaddystr = WiFi.localIP().toString();
    String returnStr = "<html><body><B>Jiggled Pod Sled</b><br><a href='http://"+ipaddystr+"'>Click here to return home</a>";
    request->send_P(200, "text/html", returnStr.c_str());
  });

  server.on("/podSledReset", HTTP_GET, [](AsyncWebServerRequest *request){
    podSledServo.write(20);
    request->send_P(200, "text/html", "Jiggled Pod Sled");
  });

  server.on("/checkMachineOffStatus", HTTP_GET, [](AsyncWebServerRequest *request){
    // checks if coffee machine is off
    //checkCoffeeMachineOffStatus();
    request->send_P(200, "text/html", "Sent request to check Coffee Machine Off Status");
  });

  server.on("/ledDataRun", HTTP_GET, [](AsyncWebServerRequest *request){
    // checks if coffee machine is off
    //outputLEDValsToCoffeeMachineSerial();
    request->send_P(200, "text/html", "Request Coffee proc to output to serial the LED color values");
  });

    server.on("/turnOnMachineIfOff", HTTP_GET, [](AsyncWebServerRequest *request){
    // checks if coffee machine is off
    //turnOnMachineIfOff();
    request->send_P(200, "text/html", "Coffee Maker command to turn off if off issued");
  });

  server.on("/turnOffMachine", HTTP_GET, [](AsyncWebServerRequest *request){
    // checks if coffee machine is off
    //turnOffMachine();
    request->send_P(200, "text/html", "Machine Turned Off");
  });

   server.on("/isMachineOff", HTTP_GET, [](AsyncWebServerRequest *request){
    // checks if coffee machine is off
    if(machineStatusFresh) {
      // Wait for an update on the machine status
       if(machineOff) {request->send_P(200, "text/html", "true");}
       else { request->send_P(200, "text/html", "false");}
       machineStatusFresh = false;
    }
    else { request->send_P(200, "text/html", "Machine Status Not Refreshed Yet"); }
     
  });
    

  server.on("/loadPod", HTTP_GET, [](AsyncWebServerRequest *request){
    //loadPod();
    request->send_P(200, "text/html", "Pod Sled loaded and jiggled");
  });


   server.on("/sledStepperBack", HTTP_GET, [](AsyncWebServerRequest *request){
    //sledStepperBack();
    request->send_P(200, "text/html", "Pod Sled loaded and jiggled");
  });
  
  
  server.on("/pressTopButton", HTTP_GET, [](AsyncWebServerRequest *request){
    if(pressTopButton()) request->send_P(200, "text/html", "Press Top Button Success");
    else request->send_P(200, "text/html", "Press Top Buttonn Command Fail");
  });

    server.on("/pressDownButton", HTTP_GET, [](AsyncWebServerRequest *request){
    if(pressDownButton()) request->send_P(200, "text/html", "Press Down Button Success");
    else request->send_P(200, "text/html", "Press Down Button Command Fail");
  });

      server.on("/pressUpButton", HTTP_GET, [](AsyncWebServerRequest *request){
    if(pressUpButton()) request->send_P(200, "text/html", "Press Up Button Success");
    else request->send_P(200, "text/html", "Press Up Button Command Fail");
  });
  
  server.on("/makeCoffee", HTTP_GET, [] (AsyncWebServerRequest *request) {
    makeCoffeeFlag = true;
    request->send(200, "text/plain", "Make Coffee Init Successful");
  }); 

    server.on("/makeCoffeeWithStage", HTTP_GET, [] (AsyncWebServerRequest *request) {
    //makeCoffee();
    request->send(200, "text/plain", "Make Coffee Init Successful");
  }); 

  server.on("/moveSledStepper", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String stepAmt;
    // GET input1 value on <ESP_IP>/makeCoffee?delay=<inputMessage1>
    if (request->hasParam("stepno")) {
      stepAmt = request->getParam("stepno")->value();
      stepAmtint = stepAmt.toInt();
    }
    movePodSledStepperFlag = true;
    request->send(200, "text/plain", "Move Sled Stepper Command Issued");
  }); 

 
  // Start server
  server.begin();
}

void loop(void) {
  if(movePodSledStepperFlag) {
    movePodSledStepperFlag = false;
    movePodSledAmt(stepAmtint);
  }

  if(makeCoffeeFlag) {
    makeCoffeeFlag = false;
    pressUpButton();
    delay(500);
    pressUpButton();
    delay(1000);
    podSledServo.write(20);
    movePodSledAmt(40000);
    podSledJiggle();
    movePodSledAmt(0);
    podSledServo.write(10);
    pressDownButton();
    delay(1000);
    pressTopButton();
  }
}

String converter(uint8_t str){
    return String((char )str);
}

bool pressTopButton() {
  // Send a command packet to the ESP-8266 controlling the coffee maker to press the top button
  digitalWrite(topButtonRelay, LOW); // swapped to match other buttons
  delayMicroseconds(1000000);
  digitalWrite(topButtonRelay, HIGH);
  delayMicroseconds(1000000);
  return true;
}

bool pressUpButton() {
  // Send a command packet to the ESP-8266 controlling the coffee maker to press the top button
  digitalWrite(upButtonRelay, LOW);
  delayMicroseconds(1500000);
  digitalWrite(upButtonRelay, HIGH);
  delayMicroseconds(1000000);
  return true;
}

bool pressDownButton() {
  // Send a command packet to the ESP-8266 controlling the coffee maker to press the top button
  digitalWrite(downButtonRelay, LOW);
  delayMicroseconds(1500000);
  digitalWrite(downButtonRelay, HIGH);
  delayMicroseconds(1000000);
  return true;
}

void resetSledServo() {
  podSledServo.write(20);
}

void podSledJiggle() {
  podSledServo.write(30);
  delay(1000);
  podSledServo.write(20);
  delay(1000);
  podSledServo.write(35);
  delay(1000);
  podSledServo.write(30);
}

void podSledDown() {
  podSledServo.write(10);
}

bool movePodSledAmt(int posNum) {
   // Move that number of steps
  stepper.moveTo(posNum);
  stepper.runToPosition();
  return true;
}

bool moveSledStepperIntoPodChamber() {
  stepper.moveTo(40000);
  stepper.runToPosition();
}

bool moveSledStepperOutOfPodChamber() {
  stepper.moveTo(0);
  stepper.runToPosition();
}
 
 
void turnOnMachine() {
  // Do two up lever pushes which will raise the pod chamber no matter if the machine is
  // on or not
  // Simulate turning on & opening pod chamber
  digitalWrite(upButtonRelay, LOW);
  //delay(500);
  digitalWrite(upButtonRelay, HIGH);
  //delay(500);
  //server.send(200, "text/json", "{\"response\": \"Coffee Machine is turned On\"}");
} 
