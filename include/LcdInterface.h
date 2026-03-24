#ifndef LCD_INTERFACE_H
#define LCD_INTERFACE_H

#include <LiquidCrystal_I2C.h>
#include <LcdMenu.h>
#include "Config.h"

class LcdInterface {
public:
    static LcdInterface& getInstance();

    void begin();
    void update();
    void processInput(InputEvent event);

private:
    LcdInterface();
    LiquidCrystal_I2C _lcd;

    unsigned long _lastActivity;
    bool _isIdle;

    void showIdleScreen();
};

void Task_LCD(void *pvParameters);

#endif // LCD_INTERFACE_H
