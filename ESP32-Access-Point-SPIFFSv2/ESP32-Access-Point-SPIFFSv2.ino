
/*  AsyncWebServer with soft Access Point IP:192.168.4.1   */

#include <Arduino.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h" // The ESPAsyncWebServer library allows us to configure the routes where the server will be listening for incoming HTTP requests and execute functions when a request is received on that route
#include "SPIFFS.h" // SPIFFS (Serial Peripheral Interface Flash File System) to save the files to build the web server
#include <Arduino_JSON.h> // library to make it easier to handle JSON strings
#include <AsyncTCP.h> // ????????????????????????
#include "RTClib.h" // RTC externo


RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

String year;
String day;
String month;
String hour;
String minutes;
String seconds;
String Date;




// Pot -----------------------------------------------------------------
// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6) 
const int potPin = 34;
// variable for storing the potentiometer value
int potValue = 0;
int LED = 13;
int i = 0;


//Network Credentials
const char* ssid     = "ESP32-AP-carjavi";
const char* password = "12345678";

// Set web server port number to 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables-----------------------------------------------------
// variables will be used to update sensor readings every 10000 milliseconds
unsigned long lastTime = 0;
unsigned long timerDelay = 100;


// Get Sensor Readings and return JSON object -------------------------------
String getSensorReadings(){
  readings["temperature"] = String(potValue);
  readings["humidity"] =  String(i);
  readings["time"] = String(Date);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}


// Initialize SPIFFS ---------------------------------------------------
void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
    Serial.println("SPIFFS mounted successfully");
}




void setup() {
    Serial.begin(115200); // Serial port for debugging purposes
    delay(100);

    pinMode(LED, OUTPUT); // Output
    

    // RTC
    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);
    }
    rtc.adjust(DateTime(__DATE__, __TIME__));
    
    
    // Initialize filesystem
    initSPIFFS();
  
    // Access Point 192.168.4.1 --------------------------------------------------
    Serial.print("Setting AP (Access Point)…");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Web Server Root URL-------------------------------------------------------
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", "text/html");
    });

     // Route to set GPIO to HIGH
    server.on("/led2on", HTTP_GET, [](AsyncWebServerRequest *request){
      digitalWrite(LED, HIGH);
      request->send(SPIFFS, "/index.html", "text/html");
    });

    // Route to set GPIO to LOW
    server.on("/led2off", HTTP_GET, [](AsyncWebServerRequest *request){
      digitalWrite(LED, LOW);
      request->send(SPIFFS, "/index.html", "text/html");
    });

    // Route to set GPIO to LOW
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
      i = 0;
      request->send(SPIFFS, "/index.html", "text/html");
    });
    
    server.serveStatic("/", SPIFFS, "/");

    // Request for the latest sensor readings-------------------------------------
    server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
      String json = getSensorReadings();
      request->send(200, "application/json", json);
      json = String();
    });

    // Events ??????????????????---------------------------------------------------------------------
    events.onConnect([](AsyncEventSourceClient *client){
      if(client->lastId()){
        Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
      }
      // send event with message "hello!", id current millis
      // and set reconnect delay to 1 second
      client->send("hello!", NULL, millis(), 10000);
    });
    server.addHandler(&events);
  
    
    // Start server-------------------------------------------------------------
    server.begin();
}




void loop(){

    // Reading potentiometer value
    potValue = analogRead(potPin);

    DateTime now = rtc.now();
    month = now.month();
    day = now.day();
    year = now.year();
    hour = now.hour();
    minutes = now.minute();
    seconds = now.second();
    
    Date = daysOfTheWeek[now.dayOfTheWeek()];
    Date = Date + " " + year + "/" + month  + "/" + day + " " + hour + ":" + minutes + ":" + seconds;
    //Serial.println(Date); 

    if ((millis() - lastTime) > timerDelay) {
      // Send Events to the client with the Sensor Readings Every 10 seconds
      events.send("ping",NULL,millis());
      events.send(getSensorReadings().c_str(),"new_readings" ,millis());
      lastTime = millis();
      i = i + 1;
      if(i > 9000){
        i = 0;
      } 
    }

}
