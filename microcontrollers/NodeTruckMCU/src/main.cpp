#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// MQTT Broker informasjon
#include "secrets.h" // Inneholder mqtt_server, mqtt_username, mqtt_password
#include "Arduino.h"
const int mqtt_port = 8883;
#define MQTT_MAX_PACKET_SIZE 256


// Definer GPIO-pinnene for hver operasjon
const int FORWARD_PIN = D1;   // Endre til riktig pinne
const int BACKWARD_PIN = D2;  // Endre til riktig pinne
const int TURNLEFT_PIN = D3;  // Endre til riktig pinne
const int TURNRIGHT_PIN = D4; // Endre til riktig pinne

WiFiClientSecure espClient;
PubSubClient client(espClient);

// Variabel for å holde styr på siste kommando
unsigned long lastCommandTime = 0;

// Variabel for å håndtere heartbeats
unsigned long lastHeartbeatTime = 0;
const unsigned long HEARTBEAT_INTERVAL = 10000; // 10 sekunder

void PostWifiTelemetry()
{
  size_t size = 128;

  Serial.print("Pakker wifi telemetri: ");

  DynamicJsonDocument doc(size);
  doc["wifi"]["rssi"] = WiFi.RSSI();
  doc["wifi"]["ip"] = WiFi.localIP().toString();        // Hent IP-adresse
  doc["wifi"]["mac"] = WiFi.macAddress();               // Hent MAC-adressen til ESP8266
  doc["wifi"]["ssid"] = WiFi.SSID();                    // Hent SSID for det tilkoblede WiFi-nettverket
  doc["wifi"]["gateway"] = WiFi.gatewayIP().toString(); // Hent Gateway-adresse
  doc["wifi"]["bssid"] = WiFi.BSSIDstr();               // Hent BSSID (MAC-adressen til tilgangspunktet)
  doc["wifi"]["channel"] = WiFi.channel();              // Hent WiFi-kanalen enheten er koblet til

  String jsonString;
  serializeJson(doc, jsonString);

  // Send JSON-meldingen via MQTT
  Serial.print("Sender wifi telemetri: ");
  Serial.println(jsonString);
  Serial.println(jsonString.length());
  client.publish("car/telemetry", jsonString.c_str());
}

void PostBaseTelemetry()
{
  size_t size = 256;

  Serial.print("Pakker Base telemetri: ");
  unsigned long now = millis();
  unsigned long uptime = now / 1000;                                   // Systemoppetid i sekunder
  unsigned long timeSinceLastCommand = (now - lastCommandTime) / 1000; // Tid siden siste kommando i sekunder

  DynamicJsonDocument doc(size);

  doc["uptime"] = uptime;
  doc["time_since_last_command"] = timeSinceLastCommand;

  // Serialisere JSON til en streng

  String jsonString;
  serializeJson(doc, jsonString);

  // Send JSON-meldingen via MQTT
  Serial.print("Sender base telemetri: ");
  Serial.println(jsonString);
  Serial.println(jsonString.length());
  client.publish("car/telemetry", jsonString.c_str());
}

void PostEspTelemetry()
{
  size_t size = 128;

  Serial.print("Pakker esp telemetri: ");

  DynamicJsonDocument doc(size);

  // doc["esp"]["vcc"] = ESP.getVcc();
  doc["esp"]["freeHeap"] = ESP.getFreeHeap();
  doc["esp"]["chipId"] = ESP.getChipId();                                           // Hent ESP-brikke ID
  doc["esp"]["flashChipSize"] = ESP.getFlashChipRealSize();                         // Få flashstørrelse
  doc["esp"]["flashChipMode"] = (ESP.getFlashChipMode() == FM_QIO) ? "QIO" : "DIO"; // Få Flash Chip Mode

  String jsonString;
  serializeJson(doc, jsonString);

  // Send JSON-meldingen via MQTT
  Serial.print("Sender esp telemetri: ");
  Serial.println(jsonString);
  Serial.println(jsonString.length());
  client.publish("car/telemetry", jsonString.c_str());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Melding mottatt [");
  Serial.print(topic);
  Serial.print("]: ");

  // Konverter payload til en null-terminert streng
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++)
  {
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
  if (msgString == "Forwards")
  {
    Serial.println("Kjører forover");
    digitalWrite(FORWARD_PIN, LOW);
    // Sett de andre pinnene til LOW
    digitalWrite(BACKWARD_PIN, HIGH);
    digitalWrite(TURNLEFT_PIN, HIGH);
    digitalWrite(TURNRIGHT_PIN, HIGH);
  }
  else if (msgString == "Backwards")
  {
    Serial.println("Kjører bakover");
    digitalWrite(BACKWARD_PIN, LOW);
    digitalWrite(FORWARD_PIN, HIGH);
    digitalWrite(TURNLEFT_PIN, HIGH);
    digitalWrite(TURNRIGHT_PIN, HIGH);
  }
  else if (msgString == "Forwards+Left")
  {
    Serial.println("Kjørerer til venstre");
    digitalWrite(TURNLEFT_PIN, LOW);
    digitalWrite(FORWARD_PIN, LOW);
    digitalWrite(BACKWARD_PIN, HIGH);
    digitalWrite(TURNRIGHT_PIN, HIGH);
  }
  else if (msgString == "Forwards+Right")
  {
    Serial.println("Kjørerer til høyre");
    digitalWrite(TURNRIGHT_PIN, LOW);
    digitalWrite(FORWARD_PIN, LOW);
    digitalWrite(BACKWARD_PIN, HIGH);
    digitalWrite(TURNLEFT_PIN, HIGH);
  }
  else if (msgString == "Backwards+Left")
  {
    Serial.println("Rygger til venstre");
    digitalWrite(TURNLEFT_PIN, LOW);
    digitalWrite(FORWARD_PIN, HIGH);
    digitalWrite(BACKWARD_PIN, LOW);
    digitalWrite(TURNRIGHT_PIN, HIGH);
  }
  else if (msgString == "Backwards+Right")
  {
    Serial.println("Rygger til høyre");
    digitalWrite(TURNRIGHT_PIN, LOW);
    digitalWrite(FORWARD_PIN, HIGH);
    digitalWrite(BACKWARD_PIN, LOW);
    digitalWrite(TURNLEFT_PIN, HIGH);
  }
  else if (msgString == "Left")
  {
    Serial.println("Svinger til venstre");
    digitalWrite(TURNLEFT_PIN, LOW);
    digitalWrite(FORWARD_PIN, HIGH);
    digitalWrite(BACKWARD_PIN, HIGH);
    digitalWrite(TURNRIGHT_PIN, HIGH);
  }
  else if (msgString == "Right")
  {
    Serial.println("Svinger til høyre");
    digitalWrite(TURNRIGHT_PIN, LOW);
    digitalWrite(FORWARD_PIN, HIGH);
    digitalWrite(BACKWARD_PIN, HIGH);
    digitalWrite(TURNLEFT_PIN, HIGH);
  }
  else if (msgString == "Ping")
  {
    PostBaseTelemetry();
    PostEspTelemetry();
    PostWifiTelemetry();
  }
  else if (msgString == "Stop")
  {
    Serial.println("Stopper");
    // Sett alle pinner til LOW for å stoppe
    StopCar();
  }
  else
  {
    Serial.println("Ukjent kommando");
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Forsøker MQTT-tilkobling...");
    String clientId = "ESP8266Client-";
    clientId += String(ESP.getChipId(), HEX);

    // Koble til med brukernavn og passord
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Tilkoblet");
      client.subscribe("car/control"); // Abonner på ønsket emne
    }
    else
    {
      Serial.print("Feilet, rc=");
      Serial.print(client.state());
      Serial.println(". Prøver igjen om 5 sekunder");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600);

  // WiFi-tilkobling med WiFiManager
  WiFiManager wm;
  wm.setTimeout(30);
  wm.setHostname("NodeTruck");
  if (!wm.autoConnect("NodeTruck", "password"))
  {
    Serial.println("Kunne ikke koble til WiFi");
    ESP.restart();
  }
  else
  {
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
  StopCar();

  // Initialiser tidspunktet for siste kommando og heartbeat
  lastCommandTime = millis();
  lastHeartbeatTime = millis();
  PostBaseTelemetry();
  PostEspTelemetry();
  PostWifiTelemetry();
}

void StopCar()
{
  digitalWrite(FORWARD_PIN, HIGH);
  digitalWrite(BACKWARD_PIN, HIGH);
  digitalWrite(TURNLEFT_PIN, HIGH);
  digitalWrite(TURNRIGHT_PIN, HIGH);
}
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();

  // Sjekk om vi skal sende en heartbeat
  if (now - lastHeartbeatTime > HEARTBEAT_INTERVAL)
  {
    lastHeartbeatTime = now;
    PostBaseTelemetry();
  }

  // Sjekk om det har gått mer enn 1 sekund siden siste kommando
  if (now - lastCommandTime > 1000)
  {
    // Stopper bilen
    Serial.println("Ingen kommando mottatt på 1 sekund, stopper bilen.");
    StopCar();

    // Oppdaterer lastCommandTime for å unngå gjentatte stoppkommandoer
    lastCommandTime = now;
  }
}