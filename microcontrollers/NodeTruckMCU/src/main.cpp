#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// MQTT Broker informasjon
#include "secrets.h" // Inneholder mqtt_server, mqtt_username, mqtt_password
const int mqtt_port = 8883;

// Definer GPIO-pinnene for hver operasjon
const int FORWARD_PIN = D1;     // Endre til riktig pinne
const int BACKWARD_PIN = D2;    // Endre til riktig pinne
const int TURNLEFT_PIN = D3;    // Endre til riktig pinne
const int TURNRIGHT_PIN = D4;   // Endre til riktig pinne

WiFiClientSecure espClient;
PubSubClient client(espClient);

// Variabel for å holde styr på siste kommando
unsigned long lastCommandTime = 0;

// Variabel for å håndtere heartbeats
unsigned long lastHeartbeatTime = 0;
const unsigned long HEARTBEAT_INTERVAL = 10000; // 10 sekunder

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Melding mottatt [");
  Serial.print(topic);
  Serial.print("]: ");

  // Konverter payload til en null-terminert streng
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
    Serial.print((char)payload[i]);
  }
  message[length] = '\0'; // Null-terminere strengen
  Serial.println();

  // Oppdaterer tidspunktet for siste kommando
  lastCommandTime = millis();

  // Konverter til String for enklere sammenligning
  String msgString = String(message);

  // Styre GPIO-pinner basert på meldingsinnhold
  if (msgString == "Forwards") {
    Serial.println("Beveger forover");
    digitalWrite(FORWARD_PIN, HIGH);
    // Sett de andre pinnene til LOW
    digitalWrite(BACKWARD_PIN, LOW);
    digitalWrite(TURNLEFT_PIN, LOW);
    digitalWrite(TURNRIGHT_PIN, LOW);
  } else if (msgString == "Backwards") {
    Serial.println("Beveger bakover");
    digitalWrite(BACKWARD_PIN, HIGH);
    digitalWrite(FORWARD_PIN, LOW);
    digitalWrite(TURNLEFT_PIN, LOW);
    digitalWrite(TURNRIGHT_PIN, LOW);
  } else if (msgString == "TurnLeft") {
    Serial.println("Svinger til venstre");
    digitalWrite(TURNLEFT_PIN, HIGH);
    digitalWrite(FORWARD_PIN, LOW);
    digitalWrite(BACKWARD_PIN, LOW);
    digitalWrite(TURNRIGHT_PIN, LOW);
  } else if (msgString == "TurnRight") {
    Serial.println("Svinger til høyre");
    digitalWrite(TURNRIGHT_PIN, HIGH);
    digitalWrite(FORWARD_PIN, LOW);
    digitalWrite(BACKWARD_PIN, LOW);
    digitalWrite(TURNLEFT_PIN, LOW);
  } else if (msgString == "Stop") {
    Serial.println("Stopper");
    // Sett alle pinner til LOW for å stoppe
    digitalWrite(FORWARD_PIN, LOW);
    digitalWrite(BACKWARD_PIN, LOW);
    digitalWrite(TURNLEFT_PIN, LOW);
    digitalWrite(TURNRIGHT_PIN, LOW);
  } else {
    Serial.println("Ukjent kommando");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Forsøker MQTT-tilkobling...");
    String clientId = "ESP8266Client-";
    clientId += String(ESP.getChipId(), HEX);

    // Koble til med brukernavn og passord
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Tilkoblet");
      client.subscribe("car/control"); // Abonner på ønsket emne
    } else {
      Serial.print("Feilet, rc=");
      Serial.print(client.state());
      Serial.println(". Prøver igjen om 5 sekunder");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  // WiFi-tilkobling med WiFiManager
  WiFiManager wm;
  if (!wm.autoConnect("AutoConnectAP", "password")) {
    Serial.println("Kunne ikke koble til WiFi");
    ESP.restart();
  } else {
    Serial.println("Tilkoblet til WiFi");
  }

  // MQTT-klientoppsett
  espClient.setInsecure(); // Bruk dette hvis du ikke legger til CA-sertifikat
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Initialiser GPIO-pinnene
  pinMode(FORWARD_PIN, OUTPUT);
  pinMode(BACKWARD_PIN, OUTPUT);
  pinMode(TURNLEFT_PIN, OUTPUT);
  pinMode(TURNRIGHT_PIN, OUTPUT);

  // Sett alle pinner til LOW ved start
  digitalWrite(FORWARD_PIN, LOW);
  digitalWrite(BACKWARD_PIN, LOW);
  digitalWrite(TURNLEFT_PIN, LOW);
  digitalWrite(TURNRIGHT_PIN, LOW);

  // Initialiser tidspunktet for siste kommando og heartbeat
  lastCommandTime = millis();
  lastHeartbeatTime = millis();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();

  // Sjekk om vi skal sende en heartbeat
  if (now - lastHeartbeatTime > HEARTBEAT_INTERVAL) {
    lastHeartbeatTime = now;
    String heartbeatMsg = "NodeCar is alive";
    Serial.print("Sender heartbeat: ");
    Serial.println(heartbeatMsg);
    client.publish("car/heartbeat", heartbeatMsg.c_str());
  }

  // Sjekk om det har gått mer enn 1 sekund siden siste kommando
  if (now - lastCommandTime > 1000) {
    // Stopper bilen
    Serial.println("Ingen kommando mottatt på 1 sekund, stopper bilen.");
    digitalWrite(FORWARD_PIN, LOW);
    digitalWrite(BACKWARD_PIN, LOW);
    digitalWrite(TURNLEFT_PIN, LOW);
    digitalWrite(TURNRIGHT_PIN, LOW);

    // Oppdaterer lastCommandTime for å unngå gjentatte stoppkommandoer
    lastCommandTime = now;
  }
}
