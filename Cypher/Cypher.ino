// Import necessary libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_camera.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials (To be configured by user)
const char* ssid = "<WIFI_SSID>";
const char* password = "<WIFI_PASSWORD>";

// API endpoints
String server = "<DEFAULT_API_ENDPOINT>";
const char* thingSpeakAPI = "<API_ENDPOINT_TO_FETCH_IP>";

// Define camera model
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Web server setup
WebServer esp32Server(80);

// Timer variables
unsigned long previousMillis = 0;
const unsigned long resetInterval = 28800000;  // 8 hours in milliseconds

// Function prototypes
void setupCamera();
void handleCameraRequest();
void fetchCurrentIP();

void setup() {
  Serial.begin(115200);
  Serial.println();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Fetch current API endpoint
  fetchCurrentIP();

  // Initialize camera
  setupCamera();

  // Define HTTP routes
  esp32Server.on("/start-camera", handleCameraRequest);

  // Start server
  esp32Server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  esp32Server.handleClient();
  
  // Reset every 8 hours
  if (millis() - previousMillis >= resetInterval) {
    Serial.println("Resetting ESP32...");
    ESP.restart();
  }
}

// Initialize camera
void setupCamera() {
  camera_config_t config;
  // Set camera pin configurations
  // ... (Configuration setup)
  
  // Initialize camera
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera initialization failed");
    while (true);
  }
  Serial.println("Camera initialized");
}

// Fetch current API endpoint
void fetchCurrentIP() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(thingSpeakAPI);
    
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      StaticJsonDocument<200> jsonBuffer;
      if (!deserializeJson(jsonBuffer, response)) {
        const char* ip = jsonBuffer["field3"];
        if (ip) {
          server = "http://" + String(ip) + ":5000/detect";
          Serial.println("Updated server IP: " + server);
        }
      }
    }
    http.end();
  }
}

// Handle incoming camera request
void handleCameraRequest() {
  Serial.println("Camera request received");
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(server);
    
    // Capture image
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    
    // Process image and send to API
    // ... (Image processing and HTTP request setup)
    
    esp_camera_fb_return(fb);
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}
