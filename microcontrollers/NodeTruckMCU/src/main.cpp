#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "secrets.h"

// MQTT Broker informasjon
const int mqtt_port = 8883;
const char* mqtt_server = "mqttbroker";
const char* mqtt_username = "username";
const char* mqtt_password = "password";

WiFiClientSecure espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Melding mottatt [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Forsøker MQTT-tilkobling...");
    String clientId = "ESP8266Client-";
    clientId += String(ESP.getChipId(), HEX);

    // Koble til med brukernavn og passord
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("tilkoblet");
      client.subscribe("inTopic"); // Abonner på ønskede emner
    } else {
      Serial.print("Feilet, rc=");
      Serial.print(client.state());
      Serial.println(". Prøver igjen om 5 sekunder");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // WiFi-tilkobling med WiFiManager
  WiFiManager wm;
  if (!wm.autoConnect("AutoConnectAP", "password")) {
    Serial.println("Kunne ikke koble til WiFi");
    ESP.restart();
  } else {
    Serial.println("Tilkoblet til WiFi");
  }

  // Legg til CA-sertifikatet hvis nødvendig
  // espClient.setCACert(ca_cert);

  // MQTT-klientoppsett
  espClient.setInsecure(); // Bruk dette hvis du ikke legger til CA-sertifikat
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "hello world #%d", value);
    Serial.print("Publiserer melding: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}
