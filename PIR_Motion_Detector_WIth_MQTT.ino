#include <odroid_go.h>

#define PIN_BLUE_LED  2
#define PIN_PIR_INPUT 4

int prePirValue;
int pirValue;
int sensorBlockingTimeout;

#include <WiFi.h>
// https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>

// get secrets from separate file
// there's arduino_secrets.example.h you can copy and rename to get started
#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

IPAddress broker(MQTT_IP);
char mqttUser[] = MQTT_USER;
char mqttPassword[] =  MQTT_PASSWORD;
int mqttPort = MQTT_PORT;

WiFiClient wifiClient;
PubSubClient client(broker, mqttPort, wifiClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      // Possible values for client.state()
      // #define MQTT_CONNECTION_TIMEOUT     -4
      // #define MQTT_CONNECTION_LOST        -3
      // #define MQTT_CONNECT_FAILED         -2
      // #define MQTT_DISCONNECTED           -1
      // #define MQTT_CONNECTED               0
      // #define MQTT_CONNECT_BAD_PROTOCOL    1
      // #define MQTT_CONNECT_BAD_CLIENT_ID   2
      // #define MQTT_CONNECT_UNAVAILABLE     3
      // #define MQTT_CONNECT_BAD_CREDENTIALS 4
      // #define MQTT_CONNECT_UNAUTHORIZED    5
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void sendDetectedStatus(int detected) {
  if (!client.connected()) {
    reconnect();
  }
  client.publish("arduino/motion_detected", detected == 1 ? "1" : "0");
}

void setup() { 
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();
  
  delay(60000);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  loopPIR();
}

void loopPIR() {
    pirValue = digitalRead(PIN_PIR_INPUT);
    
    if (prePirValue != pirValue) {
        Serial.print("PIR value: ");
        Serial.println(pirValue);
        prePirValue = pirValue;
        sendDetectedStatus(pirValue);
    }

    digitalWrite(PIN_BLUE_LED, pirValue);
}
