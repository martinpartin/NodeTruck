#include "mock_arduino.h"
#include <chrono>
#include <thread>
#include <iomanip>

// Initialiser mock-variabler
std::map<int, int> pin_states;
unsigned long mock_millis = 0;

// Mock-implementasjoner av Arduino-funksjoner

void pinMode(int pin, int mode) {
    // Ingen handling nødvendig for mocking
}

void digitalWrite(int pin, int value) {
    pin_states[pin] = value;
}

unsigned long millis() {
    return mock_millis;
}

void delay(unsigned long ms) {
    mock_millis += ms;
}

MockSerial Serial;

void MockSerial::begin(int baud) {
    // Ingen handling nødvendig for mocking
}

void MockSerial::print(const char* message) {
    std::cout << message;
}

void MockSerial::println(const char* message) {
    std::cout << message << std::endl;
}

void MockSerial::print(const String& message) {
    std::cout << message.c_str();
}

void MockSerial::println(const String& message) {
    std::cout << message.c_str() << std::endl;
}

void MockSerial::print(int value) {
    std::cout << value;
}

void MockSerial::println(int value) {
    std::cout << value << std::endl;
}

void MockSerial::print(unsigned long value) {
    std::cout << value;
}

void MockSerial::println(unsigned long value) {
    std::cout << value << std::endl;
}

void MockSerial::println() {
    std::cout << std::endl;
}

// Implementering av Mock String-klasse

String::String() : value("") {}

String::String(const char* cstr) : value(cstr) {}

// Fjernet konstruktøren som tar std::string

String::String(const String& other) : value(other.value) {}

String::String(unsigned int val, unsigned char base) {
    char buf[17]; // Nok plass for 32-bits tall i binær
    if (base == 10) {
        snprintf(buf, sizeof(buf), "%u", val);
    } else if (base == 16) {
        snprintf(buf, sizeof(buf), "%x", val);
    } else {
        buf[0] = '\0';
    }
    value = buf;
}

String& String::operator=(const char* cstr) {
    value = cstr;
    return *this;
}

String& String::operator=(const String& other) {
    value = other.value;
    return *this;
}

String& String::operator+=(const char* cstr) {
    value += cstr;
    return *this;
}

String& String::operator+=(const String& other) {
    value += other.value;
    return *this;
}

const char* String::c_str() const {
    return value.c_str();
}

bool String::operator==(const char* cstr) const {
    return value == cstr;
}

bool String::operator==(const String& other) const {
    return value == other.value;
}

bool String::operator!=(const char* cstr) const {
    return value != cstr;
}

bool String::operator!=(const String& other) const {
    return value != other.value;
}

// Implementering av MockESP

MockESP ESP;

uint32_t MockESP::getChipId() {
    return 0xDEADBEEF; // Returnerer en mock-chip-ID
}

void MockESP::restart() {
    // Ingen handling nødvendig for mocking
}

// Implementering av WiFiManager

bool WiFiManager::autoConnect(const char* apName, const char* apPassword) {
    return true; // Simulerer vellykket WiFi-tilkobling
}

// Implementering av MockWiFiClientSecure

MockWiFiClientSecure espClient;

void MockWiFiClientSecure::setInsecure() {
    // Ingen handling nødvendig for mocking
}

void MockWiFiClientSecure::setCACert(const char* ca_cert) {
    // Ingen handling nødvendig for mocking
}

// Implementering av MockPubSubClient

MockPubSubClient client(espClient);

// Lagre callback-funksjonen
static MQTT_CALLBACK_SIGNATURE callback_function = nullptr;

MockPubSubClient::MockPubSubClient(MockWiFiClientSecure& client) {
    // Ingen handling nødvendig for mocking
}

void MockPubSubClient::setServer(const char* domain, uint16_t port) {
    // Ingen handling nødvendig for mocking
}

void MockPubSubClient::setCallback(MQTT_CALLBACK_SIGNATURE callback) {
    callback_function = callback;
}

bool MockPubSubClient::connected() {
    return true; // Simulerer alltid tilkoblet
}

bool MockPubSubClient::connect(const char* clientId, const char* username, const char* password) {
    return true; // Simulerer vellykket tilkobling
}

void MockPubSubClient::subscribe(const char* topic) {
    // Ingen handling nødvendig for mocking
}

bool MockPubSubClient::publish(const char* topic, const char* payload) {
    // Loggfør publiseringen for testformål
    std::cout << "Mock publish to topic: " << topic << ", payload: " << payload << std::endl;
    return true;
}

void MockPubSubClient::loop() {
    // Ingen handling nødvendig for mocking
}

int MockPubSubClient::state() {
    return 0; // Returnerer en mock-tilstand
}
