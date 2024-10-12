#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include <WiFiClientSecure.h> 

#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// MQTT Broker informasjon
#include "secrets.h" // Inneholder mqtt_server, mqtt_username, mqtt_password
#include "Arduino.h"
const int mqtt_port = 8883;
#define MQTT_MAX_PACKET_SIZE 512


#define FIRMWARE_VERSION "0.0.4"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__


// Definer GPIO-pinnene for hver operasjon
#if defined(ESP8266)
  const int FORWARD_PIN = D1;   // Endre til riktig pinne for ESP8266
  const int BACKWARD_PIN = D2;  
  const int TURNLEFT_PIN = D3;  
  const int TURNRIGHT_PIN = D4;
#elif defined(ESP32)
  const int FORWARD_PIN = 18;   // Endre til riktig pinne for ESP32
  const int BACKWARD_PIN = 19;  
  const int TURNLEFT_PIN = 21;  
  const int TURNRIGHT_PIN = 22;
#endif

bool _carIsMoving = true;
bool _stearingIsMoving = true;

String clientId = "Client-";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// Variabel for å holde styr på siste kommando
unsigned long lastThrottleCommandTime = 0;
unsigned long lastStearingCommandTime = 0;
unsigned long lastReconnectTime = 0;
int numberOfResets = 0;

// Variabel for å håndtere heartbeats
unsigned long lastHeartbeatTime = 0;
const unsigned long HEARTBEAT_INTERVAL = 30000; // 10 sekunder

void StopCar()
{
  digitalWrite(FORWARD_PIN, HIGH);
  digitalWrite(BACKWARD_PIN, HIGH);
  digitalWrite(TURNLEFT_PIN, HIGH);
  digitalWrite(TURNRIGHT_PIN, HIGH);
  _carIsMoving = false;
  _stearingIsMoving = false;
  Serial.println("Stopper bilen..");
}

void StopCarStearing()
{
  digitalWrite(TURNLEFT_PIN, HIGH);
  digitalWrite(TURNRIGHT_PIN, HIGH);
  _stearingIsMoving = false;
  Serial.println("Resetter styring bilen..");
}

void PostWifiTelemetry()
{
  size_t size = 512;

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
  client.publish("telemetry/wifi", jsonString.c_str());
}

void PostBaseTelemetry()
{
  size_t size = 512;

  Serial.print("Pakker Base telemetri: ");
  unsigned long now = millis();
  unsigned long uptime = now / 1000;                                   // Systemoppetid i sekunder
  unsigned long timeSinceLastCommand = (now - lastThrottleCommandTime) / 1000; // Tid siden siste kommando i sekunder
  unsigned long timeSincelastReconnectTime = (now - lastReconnectTime) / 1000;


  DynamicJsonDocument doc(size);

  doc["uptime"] = uptime;
  doc["time_since_last_command"] = timeSinceLastCommand;
  doc["time_since_last_reconnect"] = timeSincelastReconnectTime;
  doc["time_since_last_numberOfReconnects"] = numberOfResets;
  

  // Serialisere JSON til en streng

  String jsonString;
  serializeJson(doc, jsonString);

  // Send JSON-meldingen via MQTT
  Serial.print("Sender base telemetri: ");
  Serial.println(jsonString);
  Serial.println(jsonString.length());
  client.publish("telemetry/base", jsonString.c_str());
}

void PostEspTelemetry()
{
  size_t size = 512;

  Serial.print("Pakker esp telemetri: ");

  DynamicJsonDocument doc(size);

  // doc["esp"]["vcc"] = ESP.getVcc();


#if defined(ESP8266)
  doc["esp"]["type"] = "ESP8266";
  doc["esp"]["reset_reason"] = ESP.getResetReason();
  doc["esp"]["reset_info"] = ESP.getResetInfo();
#elif defined(ESP32)
  doc["esp"]["type"] = "ESP32";
  doc["esp"]["reset_reason"] = (int)esp_reset_reason();  // Hent reset årsak for ESP32
#endif

  doc["code"]["firmware_version"] = FIRMWARE_VERSION;
  doc["code"]["build_date"] = BUILD_DATE;
  doc["code"]["build_time"] = BUILD_TIME;
  

  String jsonString;
  serializeJson(doc, jsonString);

  // Send JSON-meldingen via MQTT
  Serial.print("Sender esp telemetri: ");
  Serial.println(jsonString);
  Serial.println(jsonString.length());
  client.publish("telemetry/esp", jsonString.c_str());
  Serial.println("postet esp telemetri..");
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


  // Konverter til String for enklere sammenligning
  String msgString = String(message);

  // Styre GPIO-pinner basert på meldingsinnhold
  if (msgString == "Forwards")
  {
    _carIsMoving = true;
    Serial.println("Kjører forover");
    digitalWrite(FORWARD_PIN, LOW);
    digitalWrite(BACKWARD_PIN, HIGH);
    lastThrottleCommandTime = millis();
  }
  else if (msgString == "Backwards")
  {
    _carIsMoving = true;
    Serial.println("Kjører bakover");
    digitalWrite(BACKWARD_PIN, LOW);
    digitalWrite(FORWARD_PIN, HIGH);
    lastThrottleCommandTime = millis();
  }
  else if (msgString == "Left")
  {
    _stearingIsMoving = true;
    Serial.println("Svinger til venstre");
    digitalWrite(TURNLEFT_PIN, LOW);
    digitalWrite(FORWARD_PIN, HIGH);
    lastStearingCommandTime = millis();

  }
  else if (msgString == "Right")
  {
    _stearingIsMoving = true;
    Serial.println("Svinger til høyre");
    digitalWrite(TURNRIGHT_PIN, LOW);
    digitalWrite(FORWARD_PIN, HIGH);
    lastStearingCommandTime = millis();

  }
  else if (msgString == "Ping")
  {
    Serial.println("Ping");
    PostBaseTelemetry();
    Serial.println("!!!PostBaseTelemetry");
    PostWifiTelemetry();
    Serial.println("!!!PostWifiTelemetry");
    PostEspTelemetry();
    Serial.println("!!!PostEspTelemetry");

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

    // Koble til med brukernavn og passord
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Tilkoblet");
      client.subscribe("car/control"); // Abonner på ønsket emne
      lastReconnectTime = millis();
      numberOfResets++;
      PostWifiTelemetry();
      PostEspTelemetry();

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

  #if defined(ESP8266)
    clientId += String(ESP.getChipId(), HEX);  // Bruk getChipId for ESP8266
  #elif defined(ESP32)
    uint64_t chipid = ESP.getEfuseMac();       // Bruk Efuse MAC for ESP32
    clientId += String((uint32_t)(chipid >> 32), HEX);  // Bruk laveste 4 byte som "chipId"
  #endif


  // WiFi-tilkobling med WiFiManager
  WiFiManager wm;
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
  lastThrottleCommandTime = millis();
  lastHeartbeatTime = millis();
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

    if (_stearingIsMoving && (now - lastStearingCommandTime > 100))
  {
    Serial.println("Stopper sving");
    StopCar();
  }


  // Sjekk om det har gått mer enn 1 sekund siden siste kommando
  if (_carIsMoving && (now - lastThrottleCommandTime > 500))
  {
    // Stopper bilen
    Serial.println("Ingen kommando mottatt på 0.5 sekund, stopper bilen.");
    StopCar();
  }


}