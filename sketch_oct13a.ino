#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11
#define LED_PIN 5
#define BUTTON_PIN 18

DHT dht(DHTPIN, DHTTYPE);

bool manualOverride = false;
bool deviceState = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  dht.begin();
  Serial.println("Smart Home Energy System: Sensor Test");
}

void loop() {
  // Button toggles manual override
  if (digitalRead(BUTTON_PIN) == LOW) {
    manualOverride = !manualOverride;
    delay(300); // debounce
  }

  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT11!");
    return;
  }

  // Basic auto-control logic
  if (!manualOverride) {
    if (temperature > 29 || humidity > 67) {
      deviceState = true;   // turn ON
    } else {
      deviceState = false;  // turn OFF
    }
  }

  digitalWrite(LED_PIN, deviceState ? HIGH : LOW);

  Serial.print("Temp: "); Serial.print(temperature);
  Serial.print(" °C  Humidity: "); Serial.print(humidity);
  Serial.print(" %  Device: ");
  Serial.println(deviceState ? "ON" : "OFF");

  delay(2000);
}