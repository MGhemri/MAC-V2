#ifndef LCD_INTERFACE_H
#define LCD_INTERFACE_H

#include <LiquidCrystal_I2C.h>
#include <LcdMenu.h>
#include <MenuItem.h>
#include <ItemCommand.h>
#include <ItemSubMenu.h>
#include "Config.h"

class LcdInterface {
public:
    static LcdInterface& getInstance();

    void begin();
    void update();
    void processInput(InputEvent event);
    void showConnectionInfo();

private:
    LcdInterface();
    LiquidCrystal_I2C _lcd;

    unsigned long _lastActivity;
    bool _isIdle;
    bool _isShowingIP;

    void showIdleScreen();
};

void Task_LCD(void *pvParameters);

#endif // LCD_INTERFACE_H
