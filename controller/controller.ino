#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h"
#include "secrets.h"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setupMQTT();
  setupFanController();
  resetFanSpeed();
}

void loop() {
  mqttClientLoop();
}

void resetFanSpeed() {
  setFanSpeed(true, true, 0);
  publishFanSpeed(true, true, 0);
}