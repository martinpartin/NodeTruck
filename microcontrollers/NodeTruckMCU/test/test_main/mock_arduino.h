#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <map>
#include <string>
#include <cstdint>
#include <iostream>
#include <stdio.h>

// Mock-definisjoner for Arduino-typer og funksjoner

// Definer Arduino-datatyper
typedef unsigned char byte;

// Definer HIGH og LOW
#define HIGH 0x1
#define LOW  0x0

// Definer INPUT og OUTPUT
#define OUTPUT 0x1
#define INPUT  0x0

// Definer HEX for basiskonvertering
#define HEX 16

// Mock GPIO-pin-tilstander
extern std::map<int, int> pin_states;
extern unsigned long mock_millis;

// Mock-funksjoner
void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
unsigned long millis();
void delay(unsigned long ms);

// **Flyttet String-klassen over MockSerial**

class String {
public:
    String();
    String(const char* cstr);
    // Fjernet konstruktøren som tar std::string
    String(const String& other);
    String(unsigned int val, unsigned char base = 10);

    String& operator=(const char* cstr);
    String& operator=(const String& other);

    String& operator+=(const char* cstr);
    String& operator+=(const String& other);

    const char* c_str() const;

    bool operator==(const char* cstr) const;
    bool operator==(const String& other) const;
    bool operator!=(const char* cstr) const;
    bool operator!=(const String& other) const;

private:
    std::string value;
};

// Mock Serial-klasse
class MockSerial {
public:
    void begin(int baud);
    void print(const char* message);
    void println(const char* message);
    void print(const String& message);      // Nå er String deklarert
    void println(const String& message);    // Nå er String deklarert
    void print(int value);
    void println(int value);
    void print(unsigned long value);
    void println(unsigned long value);
    void println();
    // Fjernet metoder med std::string-parameter
};

extern MockSerial Serial;

// Mock ESP-klasse
class MockESP {
public:
    uint32_t getChipId();
    void restart();
};

extern MockESP ESP;

// Mock WiFiManager-klasse
class WiFiManager {
public:
    bool autoConnect(const char* apName, const char* apPassword);
};

// Mock WiFiClientSecure-klasse
class MockWiFiClientSecure {
public:
    void setInsecure();
    void setCACert(const char* ca_cert);
};

extern MockWiFiClientSecure espClient;

// Mock PubSubClient-klasse
typedef void (*MQTT_CALLBACK_SIGNATURE)(char*, byte*, unsigned int);

class MockPubSubClient {
public:
    MockPubSubClient(MockWiFiClientSecure& client);
    void setServer(const char* domain, uint16_t port);
    void setCallback(MQTT_CALLBACK_SIGNATURE callback);
    bool connected();
    bool connect(const char* clientId, const char* username, const char* password);
    void subscribe(const char* topic);
    bool publish(const char* topic, const char* payload);
    void loop();
    int state();
};

extern MockPubSubClient client;

#endif // MOCK_ARDUINO_H
