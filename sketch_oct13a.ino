#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

//Pin Configuration

#define DHTPIN 4          // DHT11 Data Pin (D4)
#define DHTTYPE DHT11
#define LED_PIN 5         // LED Pin (D5)

// Sensor & WiFi Setup

DHT dht(DHTPIN, DHTTYPE);

// WiFi Credentials
const char* WIFI_SSID = "NOTHING";
const char* WIFI_PASSWORD = "1234567890";

//Flask Server Endpoint
const char* SERVER_URL = "http://10.185.81.166:5000/api/data";  

bool deviceState = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();

  Serial.println("\nSmart Home Energy System - WiFi + Flask Integration");
  Serial.println("----------------------------------------------------");

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
Serial.print("Connecting to WiFi");

int retries = 0;
while (WiFi.status() != WL_CONNECTED && retries < 20) {
  delay(1000);
  Serial.print(".");
  retries++;
}
if (WiFi.status() == WL_CONNECTED) {
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
} else {
  Serial.println("\nWiFi connection failed. Restarting...");
  ESP.restart();
}

  Serial.println("\nWiFi connected successfully!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Validate sensor data
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT11 sensor!");
    delay(2000);
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" °C | Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Send data to Flask server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");

    // Prepare JSON data
    String jsonData = "{\"temperature\": " + String(temperature) +
                      ", \"humidity\": " + String(humidity) + "}";

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("Server Response: ");
      Serial.println(response);

      if (response.indexOf("ON") != -1) {
        deviceState = true;
      } else if (response.indexOf("OFF") != -1) {
        deviceState = false;
      }

      digitalWrite(LED_PIN, deviceState ? HIGH : LOW);

    } else {
      Serial.print("Error sending data. HTTP code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Wi-Fi disconnected, retrying...");
    WiFi.reconnect();
  }

  delay(5000);  // Send data to flask(backend) every 5 seconds
}