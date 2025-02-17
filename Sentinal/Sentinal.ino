#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

// Replace with your WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Replace with your ESP32-CAM IP address
const char* esp32CamIP = "YOUR_ESP32_IP";
const char* cameraEndpoint = "/start-camera";

SoftwareSerial nano(D5, D6);  // RX, TX
ESP8266WebServer server(80);

// GPIO and LED statuses
const uint8_t LED1pin = D0;
bool LED1status = LOW;
bool LED2status = LOW;

// Motion detection variables
bool motionDetected = false;
int personCount = 0;
int responseCycles = 0;

void setup() {
  Serial.begin(9600);
  nano.begin(9600);
  pinMode(LED1pin, OUTPUT);
  digitalWrite(LED1pin, LOW);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    server.on("/", handle_OnConnect);
    server.on("/led1on", handle_led1on);
    server.on("/led1off", handle_led1off);
    server.on("/led2on", handle_led2on);
    server.on("/led2off", handle_led2off);
    server.onNotFound(handle_NotFound);
    server.begin();
    Serial.println("HTTP server started");
  } else {
    Serial.println("\nWiFi connection failed.");
  }
}

void loop() {
  server.handleClient();
  digitalWrite(LED1pin, LED1status ? LOW : HIGH);

  if (nano.available()) {
    char receivedChar = nano.read();
    Serial.print("Received from Nano: ");
    Serial.println(receivedChar);

    if (receivedChar == 'M') {
      motionDetected = true;
      Serial.println("Motion detected! Communicating with ESP32...");
      communicateWithESP32();
    }
  }

  if (responseCycles >= 10) {
    if (personCount > 0) {
      Serial.println("Person detected! Checking ThingSpeak API...");
      handleThingSpeakAPI();
    } else {
      Serial.println("No person detected in 10 cycles. Sending 'l' to Nano...");
      nano.print('l');
    }
    personCount = 0;
    responseCycles = 0;
  }
}

void communicateWithESP32() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    String url = String("http://") + esp32CamIP + cameraEndpoint;
    http.begin(client, url);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("ESP32 Response: " + response);
      if (response.indexOf("person") != -1) {
        personCount++;
      }
      responseCycles++;
    }
    http.end();
  }
}

void handleThingSpeakAPI() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    String url = "https://api.thingspeak.com/channels/YOUR_CHANNEL_ID/fields/4/last.json?api_key=YOUR_API_KEY";
    http.begin(client, url);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      StaticJsonDocument<256> jsonDoc;
      if (deserializeJson(jsonDoc, response) == DeserializationError::Ok) {
        int field4Value = jsonDoc["field4"].as<int>();
        if (field4Value == 1) {
          nano.print('C');
          nano.print('L');
        } else {
          nano.print('C');
        }
      }
    }
    http.end();
  }
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(LED1status, LED2status));
}

void handle_led1on() {
  LED1status = HIGH;
  nano.print('N');
  server.send(200, "text/html", SendHTML(LED1status, LED2status));
}

void handle_led1off() {
  LED1status = LOW;
  nano.print('n');
  server.send(200, "text/html", SendHTML(LED1status, LED2status));
}

void handle_led2on() {
  LED2status = HIGH;
  nano.print('L');
  server.send(200, "text/html", SendHTML(LED1status, LED2status));
}

void handle_led2off() {
  LED2status = LOW;
  nano.print('l');
  server.send(200, "text/html", SendHTML(LED1status, LED2status));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(bool led1stat, bool led2stat) {
  String ptr = "<!DOCTYPE html><html><head><title>ESP8266 Control</title></head><body>";
  ptr += "<h1>ESP8266 Web Server</h1><h3>Control Panel</h3>";
  ptr += led1stat ? "<p>LED1: <strong>ON</strong></p><a href='/led1off'>Turn OFF</a>" : "<p>LED1: <strong>OFF</strong></p><a href='/led1on'>Turn ON</a>";
  ptr += led2stat ? "<p>LED2: <strong>ON</strong></p><a href='/led2off'>Turn OFF</a>" : "<p>LED2: <strong>OFF</strong></p><a href='/led2on'>Turn ON</a>";
  ptr += "</body></html>";
  return ptr;
}
