#include "Hardware.h"
#include "DataManager.h"
#include <Arduino.h>

QueueHandle_t Hardware::inputQueue = NULL;
QueueHandle_t Hardware::rawInputQueue = NULL;
unsigned long Hardware::lastInterruptTime = 0;
int Hardware::lastEncoderState = LOW;

Hardware::Hardware() {}

Hardware& Hardware::getInstance() {
    static Hardware instance;
    return instance;
}

void Hardware::begin() {
    // Relays
    for (int i = 0; i < MAX_RELAYS; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
    }
    updateRelays();

    // Encoder
    pinMode(PIN_ENCODER_CLK, INPUT_PULLUP);
    pinMode(PIN_ENCODER_DT, INPUT_PULLUP);
    pinMode(PIN_ENCODER_SW, INPUT_PULLUP);

    lastEncoderState = digitalRead(PIN_ENCODER_CLK);

    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_CLK), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_SW), buttonISR, FALLING);

    inputQueue = xQueueCreate(QUEUE_INPUT_SIZE, sizeof(InputEvent));
    rawInputQueue = xQueueCreate(20, sizeof(InputEvent)); // Larger buffer for raw events
}

void Hardware::updateRelays() {
    AppConfig cfg = DataManager::getInstance().getConfig();
    bool invert = (cfg.system.relay_polarity == "LOW");

    for (int i = 0; i < MAX_RELAYS; i++) {
        int state = cfg.relays[i].state;
        digitalWrite(RELAY_PINS[i], invert ? (state ? LOW : HIGH) : (state ? HIGH : LOW));
    }
}

void Hardware::toggleRelay(int id) {
    if (id < 1 || id > MAX_RELAYS) return;
    AppConfig cfg = DataManager::getInstance().getConfig();
    int currentState = cfg.relays[id - 1].state;
    DataManager::getInstance().setRelayState(id, currentState ? 0 : 1);
    updateRelays();
}

void IRAM_ATTR Hardware::encoderISR() {
    int currentState = digitalRead(PIN_ENCODER_CLK);
    if (currentState != lastEncoderState) {
        InputEvent ev;
        if (digitalRead(PIN_ENCODER_DT) != currentState) {
            ev = InputEvent::UP;
        } else {
            ev = InputEvent::DOWN;
        }
        xQueueSendFromISR(rawInputQueue, &ev, NULL);
    }
    lastEncoderState = currentState;
}

void IRAM_ATTR Hardware::buttonISR() {
    InputEvent ev = InputEvent::CLICK;
    xQueueSendFromISR(rawInputQueue, &ev, NULL);
}

void Task_Input(void *pvParameters) {
    InputEvent rawEv;
    unsigned long lastEvTime = 0;

    for (;;) {
        if (xQueueReceive(Hardware::rawInputQueue, &rawEv, portMAX_DELAY) == pdTRUE) {
            unsigned long now = millis();
            // Debounce logic: ignore events too close to each other
            if (now - lastEvTime > DEBOUNCE_TIME_MS) {
                xQueueSend(Hardware::inputQueue, &rawEv, 0);
                lastEvTime = now;
            }
        }
    }
}

void Task_Relay(void *pvParameters) {
    for (;;) {
        Hardware::getInstance().updateRelays();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
