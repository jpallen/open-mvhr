void setupWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}


void setupMQTT() { 
  setupWifi();

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(mqttCallback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ArduinoNanoESP32", MQTT_USER, MQTT_PASS)) {
      Serial.println("Connected to MQTT");
      client.subscribe(INTAKE_FAN_SPEED_TOPIC);
      client.subscribe(EXHAUST_FAN_SPEED_TOPIC);
    } else {
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  printMqttCallback(topic, payload, length);

  char message[length + 1];
  strncpy(message, (char*)payload, length);
  message[length] = '\0';
  int speed = atoi(message);
  speed = constrain(speed, 0, 10);

  bool intake = (strcmp(topic, INTAKE_FAN_SPEED_TOPIC) == 0);
  bool exhaust = (strcmp(topic, EXHAUST_FAN_SPEED_TOPIC) == 0);

  if (intake | exhaust) {
    setFanSpeed(intake, exhaust, speed);
    publishFanSpeed(intake, exhaust, speed);
  }
}

void publishFanSpeed(bool intake, bool exhaust, int speed) {
  if (intake) {
    String stateTopic = String(INTAKE_FAN_SPEED_TOPIC) + String("/state");
    client.publish(stateTopic.c_str(), String(speed).c_str(), true);
  }

  if (exhaust) {
    String stateTopic = String(EXHAUST_FAN_SPEED_TOPIC) + String("/state");
    client.publish(stateTopic.c_str(), String(speed).c_str(), true);
  }
}

void printMqttCallback(char* topic, byte* payload, unsigned int length) {
  // Print the topic
  Serial.print("Topic: ");
  Serial.println(topic);

  // Print the payload as a string
  Serial.print("Payload: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);  // Convert each byte to a char
  }
  Serial.println();

  // Print the length of the payload
  Serial.print("Payload Length: ");
  Serial.println(length);
}
