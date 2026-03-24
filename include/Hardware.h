#ifndef HARDWARE_H
#define HARDWARE_H

#include "Config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class Hardware {
public:
    static Hardware& getInstance();

    void begin();
    void updateRelays();
    void toggleRelay(int id);

    static QueueHandle_t inputQueue;
    static QueueHandle_t rawInputQueue; // New queue for raw pulses

private:
    Hardware();

    static void IRAM_ATTR encoderISR();
    static void IRAM_ATTR buttonISR();

    static unsigned long lastInterruptTime;
    static int lastEncoderState;
};

void Task_Input(void *pvParameters);
void Task_Relay(void *pvParameters);

#endif // HARDWARE_H
