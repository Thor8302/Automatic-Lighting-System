#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


SoftwareSerial nano(D5, D6);  // RX, TX


const char* ssid = "Tiger- ek bakri ki katha";
const char* password = "Amba@123";

// ESP8266 WebServer
ESP8266WebServer server(80);

// ESP32 Camera Endpoint
const char* esp32CamIP = "192.168.29.25";  // Replace with the actual IP address of the ESP32
const char* cameraEndpoint = "/start-camera";

// NTP settings
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // IST timezone offset 5:30 (19800s)

// GPIO and LED statuses
uint8_t LED1pin = D0;  // GPIO13
bool LED1status = LOW;
bool LED2status = LOW;

// Variables for motion detection
bool motionDetected = false;
int personCount = 0;        // Tracks "person" detections in AWS response
int responseCycles = 0;     // Count response cycles

void setup() {
  // Serial communication
  Serial.begin(9600);
  nano.begin(9600);

  // Configure GPIO
  pinMode(LED1pin, OUTPUT);
  digitalWrite(LED1pin, LOW);

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("NodeMCU IP: ");
    Serial.println(WiFi.localIP());

    // Start NTP client
    timeClient.begin();

    // Define HTTP server routes
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
  // Handle web server requests
  server.handleClient();
  digitalWrite(LED1pin, LED1status ? LOW : HIGH);

  // Update NTP time
  timeClient.update();
  checkResetTime();  // Check if it's time to reset

  // Check for motion detection messages from Arduino Nano
  if (nano.available()) {
    char receivedChar = nano.read();
    Serial.print("Received from Nano: ");
    Serial.println(receivedChar);

    if (receivedChar == 'M') {  // Motion detected
      motionDetected = true;
      Serial.println("Motion detected! Communicating with ESP32...");
      communicateWithESP32();
    }
  }

  // Process ESP32 responses to detect "person"
  if (responseCycles >= 10) {
    if (personCount > 0) {
      Serial.println("Person detected! Sending continue signal to Nano...");
      nano.print('C');  // Notify Arduino Nano to continue motion
    } else {
      Serial.println("No person detected in 10 cycles.");
    }

    // Reset counters
    personCount = 0;
    responseCycles = 0;
  }
}

// Function to check if it's 9:00 a.m. and trigger reset
void checkResetTime() {
  if (timeClient.getHours() == 9 && timeClient.getMinutes() == 00) {
    if(timeClient.getSeconds()<3)
    {
      Serial.println("It's 9:00 a.m.! Sending reset commands...");
      sendResetToESP32();
     // nano.print('R');  // Send reset command to Arduino Nano
      delay(1000);
      ESP.restart();    // Reset NodeMCU
    }
  }
  if (timeClient.getHours() == 21 && timeClient.getMinutes() == 00) {
    if(timeClient.getSeconds()<3)
    {
      Serial.println("It's 9:00 a.m.! Sending reset commands...");
      sendResetToESP32();
     // nano.print('R');  // Send reset command to Arduino Nano
      delay(1000);
      ESP.restart();    // Reset NodeMCU
    }
  }
}

// Function to communicate with ESP32 for motion detection
void communicateWithESP32() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;  // Create a WiFiClient object
    HTTPClient http;

    String url = String("http://") + esp32CamIP + cameraEndpoint;
    Serial.println("Sending HTTP GET request to ESP32: " + url);
    http.begin(client, url);  // Start HTTP request with client and URL

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response from ESP32: " + response);

      // Check for "person" in the response
      if (response.indexOf("person") != -1) {
        personCount++;
        Serial.println("Detected 'person' in the response");
      } else {
        Serial.println("No 'person' detected in the response");
      }

      responseCycles++;
    } else {
      Serial.printf("Error in HTTP request: %d\n", httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected. Cannot communicate with ESP32.");
  }
}

// Function to send reset command to ESP32
void sendResetToESP32() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    String url = String("http://") + esp32CamIP + "/reset";  // Assuming ESP32 has a /reset endpoint
    Serial.println("Sending reset request to ESP32: " + url);

    http.begin(client, url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("ESP32 Reset Response: " + response);
    } else {
      Serial.printf("Error in sending reset request to ESP32: %d\n", httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected. Cannot send reset request to ESP32.");
  }
}

// HTTP handler functions
void handle_OnConnect() {
  LED1status = LOW;
  LED2status = LOW;
  Serial.println("LED1 OFF | LED2 OFF");
  server.send(200, "text/html", SendHTML(LED1status, LED2status)); 
}

void handle_led1on() {
  LED1status = HIGH;
  nano.print('N');  // Notify Arduino Nano
  Serial.println("LED1 ON");
  server.send(200, "text/html", SendHTML(LED1status, LED2status)); 
}

void handle_led1off() {
  LED1status = LOW;
  nano.print('n');  // Notify Arduino Nano
  Serial.println("LED1 OFF");
  server.send(200, "text/html", SendHTML(LED1status, LED2status)); 
}

void handle_led2on() {
  LED2status = HIGH;
  nano.print('L');  // Notify Arduino Nano
  Serial.println("LED2 ON");
  server.send(200, "text/html", SendHTML(LED1status, LED2status)); 
}

void handle_led2off() {
  LED2status = LOW;
  nano.print('l');  // Notify Arduino Nano
  Serial.println("LED2 OFF");
  server.send(200, "text/html", SendHTML(LED1status, LED2status)); 
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

// HTML generation function
String SendHTML(bool led1stat, bool led2stat) {
  String ptr = "<!DOCTYPE html><html>";
  ptr += "<head>";
  ptr += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  ptr += "<title>ESP8266 Control Panel</title>";
  ptr += "<style>";
  ptr += "html { font-family: Helvetica; text-align: center;}";
  ptr += "body { margin-top: 50px; }";
  ptr += "h1 { color: #444; margin-bottom: 30px; }";
  ptr += "h3 { color: #444; margin-bottom: 50px; }";
  ptr += ".button { display: inline-block; background-color: #1abc9c; color: white; padding: 13px 30px;";
  ptr += "text-decoration: none; font-size: 20px; margin: 10px; border-radius: 4px; border: none; cursor: pointer;}";
  ptr += ".button-on { background-color: #1abc9c; }";
  ptr += ".button-on:active { background-color: #16a085; }";
  ptr += ".button-off { background-color: #e74c3c; }";
  ptr += ".button-off:active { background-color: #c0392b; }";
  ptr += "</style>";

  ptr += "</style>";
  ptr += "</head>";
  ptr += "<body>";
  ptr += "<h1>ESP8266 Web Server</h1>";
  ptr += "<h3>Control Panel</h3>";

  // LED1 controls
  if (led1stat) {
    ptr += "<p>Night Mode Status: <strong>ON</strong></p>";
    ptr += "<a class=\"button button-off\" href=\"/led1off\">Turn OFF</a>";
  } else {
    ptr += "<p>Night Mode Status: <strong>OFF</strong></p>";
    ptr += "<a class=\"button button-on\" href=\"/led1on\">Turn ON</a>";
  }

  // LED2 controls
  if (led2stat) {
    ptr += "<p>LED2 Status: <strong>ON</strong></p>";
    ptr += "<a class=\"button button-off\" href=\"/led2off\">Turn OFF</a>";
  } else {
    ptr += "<p>LED2 Status: <strong>OFF</strong></p>";
    ptr += "<a class=\"button button-on\" href=\"/led2on\">Turn ON</a>";
  }

  ptr += "</body></html>";
  return ptr;
}
