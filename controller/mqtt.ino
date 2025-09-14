// Configuration variables at the top of your file
unsigned long lastReconnectAttempt = 0;

// Timing constants
const unsigned long RECONNECT_INTERVAL = 5000;
const unsigned long WIFI_CONNECT_TIMEOUT = 10000;
const unsigned long MQTT_CONNECT_TIMEOUT = 10000;
const unsigned long CONNECTION_RETRY_DELAY = 1000;

// MQTT configuration
const int MQTT_PORT = 1883;                        // MQTT broker port
const int FAN_SPEED_MIN = 0;                       // Minimum fan speed
const int FAN_SPEED_MAX = 10;                      // Maximum fan speed

bool connectWifi() {
  // Check if already connected
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  
  Serial.println("Connecting to WiFi...");
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Wait for connection with timeout
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_CONNECT_TIMEOUT) {
    delay(CONNECTION_RETRY_DELAY);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("\nFailed to connect to WiFi");
    return false;
  }
}

void setupMQTT() { 
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);
  
  // Try to connect once during setup
  reconnectMQTT();
}

bool reconnectMQTT() {
  // Check WiFi first and attempt connection if needed
  if (!connectWifi()) {
    return false;
  }
  
  // Check if already connected to MQTT
  if (client.connected()) {
    return true;
  }
  
  Serial.println("Connecting to MQTT...");
  
  // Try to connect with timeout
  unsigned long startAttempt = millis();
  while (!client.connected() && millis() - startAttempt < MQTT_CONNECT_TIMEOUT) {
    // Create a unique client ID
    String clientId = "ArduinoNanoESP32-";
    clientId += String(random(0xffff), HEX);
    
    Serial.print("Attempting MQTT connection with ID: ");
    Serial.print(clientId);
    Serial.print("...");
    
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println(" connected!");
      
      // Resubscribe to topics
      client.subscribe(INTAKE_FAN_SPEED_TOPIC);
      client.subscribe(EXHAUST_FAN_SPEED_TOPIC);
      
      // Optionally publish a reconnection message
      client.publish("status/arduino", "connected", true);
      
      return true;
    } else {
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying...");
      delay(CONNECTION_RETRY_DELAY);
    }
  }
  
  Serial.println("MQTT connection timeout");
  return false;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  printMqttCallback(topic, payload, length);

  char message[length + 1];
  strncpy(message, (char*)payload, length);
  message[length] = '\0';
  int speed = atoi(message);
  speed = constrain(speed, FAN_SPEED_MIN, FAN_SPEED_MAX);

  bool intake = (strcmp(topic, INTAKE_FAN_SPEED_TOPIC) == 0);
  bool exhaust = (strcmp(topic, EXHAUST_FAN_SPEED_TOPIC) == 0);

  if (intake | exhaust) {
    setFanSpeed(intake, exhaust, speed);
    publishFanSpeed(intake, exhaust, speed);
  }
}

void publishFanSpeed(bool intake, bool exhaust, int speed) {
  // Check connection before publishing
  if (!client.connected()) {
    Serial.println("MQTT not connected, skipping publish");
    return;
  }
  
  if (intake) {
    String stateTopic = String(INTAKE_FAN_SPEED_TOPIC) + String("/state");
    Serial.printf("Publish %s %d", stateTopic.c_str(), speed);
    Serial.println();

    client.publish(stateTopic.c_str(), String(speed).c_str(), true);
  }

  if (exhaust) {
    String stateTopic = String(EXHAUST_FAN_SPEED_TOPIC) + String("/state");

    Serial.printf("Publish %s %d", stateTopic.c_str(), speed);
    Serial.println();
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

void mqttClientLoop() {
  // Handle MQTT reconnection
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > RECONNECT_INTERVAL) {
      lastReconnectAttempt = now;
      if (reconnectMQTT()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Process MQTT messages
    client.loop();
  }
}