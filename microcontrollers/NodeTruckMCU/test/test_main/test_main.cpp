#define UNIT_TEST

#include <mock_arduino.h> // For å inkludere Arduino-typer
#include <unity.h>
#include "main.cpp"  // Inkluderer koden som skal testes

void setUp(void) {
    // Dette kjøres før hver test
    pin_states.clear();
    mock_millis = 0;
    lastCommandTime = 0;
}

void tearDown(void) {
    // Dette kjøres etter hver test
}

// Test for "Forwards" kommando
void test_forwards_command(void) {
    // Simulerer mottak av "Forwards" kommando
    char topic[] = "car/control";
    byte payload[] = "Forwards";
    unsigned int length = strlen((char*)payload);

    callback(topic, payload, length);

    // Sjekk at FORWARD_PIN er HIGH og andre er LOW
    TEST_ASSERT_EQUAL(HIGH, pin_states[FORWARD_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[BACKWARD_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[TURNLEFT_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[TURNRIGHT_PIN]);
}

// Test for "Backwards" kommando
void test_backwards_command(void) {
    char topic[] = "car/control";
    byte payload[] = "Backwards";
    unsigned int length = strlen((char*)payload);

    callback(topic, payload, length);

    TEST_ASSERT_EQUAL(LOW, pin_states[FORWARD_PIN]);
    TEST_ASSERT_EQUAL(HIGH, pin_states[BACKWARD_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[TURNLEFT_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[TURNRIGHT_PIN]);
}

// Test for "TurnLeft" kommando
void test_turnleft_command(void) {
    char topic[] = "car/control";
    byte payload[] = "TurnLeft";
    unsigned int length = strlen((char*)payload);

    callback(topic, payload, length);

    TEST_ASSERT_EQUAL(LOW, pin_states[FORWARD_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[BACKWARD_PIN]);
    TEST_ASSERT_EQUAL(HIGH, pin_states[TURNLEFT_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[TURNRIGHT_PIN]);
}

// Test for "TurnRight" kommando
void test_turnright_command(void) {
    char topic[] = "car/control";
    byte payload[] = "TurnRight";
    unsigned int length = strlen((char*)payload);

    callback(topic, payload, length);

    TEST_ASSERT_EQUAL(LOW, pin_states[FORWARD_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[BACKWARD_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[TURNLEFT_PIN]);
    TEST_ASSERT_EQUAL(HIGH, pin_states[TURNRIGHT_PIN]);
}

// Test for "Stop" kommando
void test_stop_command(void) {
    // Først setter noen pinner til HIGH
    pin_states[FORWARD_PIN] = HIGH;

    char topic[] = "car/control";
    byte payload[] = "Stop";
    unsigned int length = strlen((char*)payload);

    callback(topic, payload, length);

    // Sjekk at alle pinner er LOW
    TEST_ASSERT_EQUAL(LOW, pin_states[FORWARD_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[BACKWARD_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[TURNLEFT_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[TURNRIGHT_PIN]);
}

// Test for automatisk stopp etter 1 sekund
void test_auto_stop(void) {
    // Simulerer mottak av "Forwards" kommando
    char topic[] = "car/control";
    byte payload[] = "Forwards";
    unsigned int length = strlen((char*)payload);

    callback(topic, payload, length);

    // Sjekk at FORWARD_PIN er HIGH
    TEST_ASSERT_EQUAL(HIGH, pin_states[FORWARD_PIN]);

    // Simulerer at det har gått 1001 ms
    mock_millis = lastCommandTime + 1001;

    // Kall loop-funksjonen
    loop();

    // Sjekk at alle pinner er LOW
    TEST_ASSERT_EQUAL(LOW, pin_states[FORWARD_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[BACKWARD_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[TURNLEFT_PIN]);
    TEST_ASSERT_EQUAL(LOW, pin_states[TURNRIGHT_PIN]);
}

// Test for heartbeat-melding
void test_heartbeat(void) {
    // Setter opp en mock for client.publish
    bool heartbeat_sent = false;
    auto original_publish = client.publish;

    client.publish = [&](const char* topic, const char* payload) -> bool {
        if (strcmp(topic, "car/heartbeat") == 0 && strcmp(payload, "NodeCar is alive") == 0) {
            heartbeat_sent = true;
        }
        return true;
    };

    // Simulerer at det har gått mer enn HEARTBEAT_INTERVAL
    mock_millis = lastHeartbeatTime + HEARTBEAT_INTERVAL + 1;

    // Kall loop-funksjonen
    loop();

    // Sjekk at heartbeat ble sendt
    TEST_ASSERT_TRUE(heartbeat_sent);

    // Tilbakestill client.publish
    client.publish = original_publish;
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_forwards_command);
    RUN_TEST(test_backwards_command);
    RUN_TEST(test_turnleft_command);
    RUN_TEST(test_turnright_command);
    RUN_TEST(test_stop_command);
    RUN_TEST(test_auto_stop);
    // RUN_TEST(test_heartbeat); // Denne testen krever ytterligere mocking av client.publish

    UNITY_END();

    return 0;
}
