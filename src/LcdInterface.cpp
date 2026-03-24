#include "LcdInterface.h"
#include "DataManager.h"
#include "Hardware.h"
#include <WiFi.h>

// Helper functions for menu
void actionToggleRelay1() { Hardware::getInstance().toggleRelay(1); }
void actionToggleRelay2() { Hardware::getInstance().toggleRelay(2); }
void actionToggleRelay3() { Hardware::getInstance().toggleRelay(3); }
void actionToggleRelay4() { Hardware::getInstance().toggleRelay(4); }
void actionToggleRelay5() { Hardware::getInstance().toggleRelay(5); }
void actionToggleRelay6() { Hardware::getInstance().toggleRelay(6); }
void actionToggleRelay7() { Hardware::getInstance().toggleRelay(7); }
void actionToggleRelay8() { Hardware::getInstance().toggleRelay(8); }
void actionReboot() { ESP.restart(); }

// Simplified structure to avoid macro issues
// Based on LcdMenu v3.x source, MenuItem is the base class
MenuItem* mainMenu[] = {
    new MenuItem("Relay 1", 1), // Using type 1 (command) if supported
    new MenuItem("Relay 2", 1),
    new MenuItem("Relay 3", 1),
    new MenuItem("Relay 4", 1),
    new MenuItem("Relay 5", 1),
    new MenuItem("Relay 6", 1),
    new MenuItem("Relay 7", 1),
    new MenuItem("Relay 8", 1),
    new MenuItem("Reboot", 1)
};

LcdMenu menu(LCD_ROWS, LCD_COLS);

LcdInterface::LcdInterface() : _lcd(LCD_ADDR, LCD_COLS, LCD_ROWS), _lastActivity(0), _isIdle(true) {}

LcdInterface& LcdInterface::getInstance() {
    static LcdInterface instance;
    return instance;
}

void LcdInterface::begin() {
    if (xSemaphoreTake(DataManager::mutexI2C, pdMS_TO_TICKS(1000)) == pdTRUE) {
        _lcd.init();
        _lcd.backlight();
        xSemaphoreGive(DataManager::mutexI2C);
    }
    menu.setupLcdWithMenu(0x27, mainMenu, 9);
    showIdleScreen();
}

void LcdInterface::showIdleScreen() {
    if (xSemaphoreTake(DataManager::mutexI2C, pdMS_TO_TICKS(1000)) == pdTRUE) {
        _lcd.clear();
        _lcd.setCursor(0, 0);
        _lcd.print("MAC-V2 Controller");
        _lcd.setCursor(0, 1);
        _lcd.print("IP: ");
        _lcd.print(WiFi.localIP());

        AppConfig cfg = DataManager::getInstance().getConfig();
        int active = 0;
        for(int i=0; i<8; i++) if(cfg.relays[i].state) active++;

        _lcd.setCursor(0, 2);
        _lcd.print("Active Relays: ");
        _lcd.print(active);
        _lcd.print("/8");

        _lcd.setCursor(0, 3);
        _lcd.print("Click for Menu");
        xSemaphoreGive(DataManager::mutexI2C);
    }
    _isIdle = true;
}

void LcdInterface::processInput(InputEvent event) {
    _lastActivity = millis();

    if (_isIdle) {
        if (event == InputEvent::CLICK) {
            _isIdle = false;
        }
        return;
    }

    if (xSemaphoreTake(DataManager::mutexI2C, pdMS_TO_TICKS(1000)) == pdTRUE) {
        switch (event) {
            case InputEvent::UP: menu.up(); break;
            case InputEvent::DOWN: menu.down(); break;
            case InputEvent::CLICK: menu.enter(); break;
            case InputEvent::LONG_CLICK: menu.back(); break;
        }
        xSemaphoreGive(DataManager::mutexI2C);
    }
}

void LcdInterface::update() {
    if (!_isIdle && (millis() - _lastActivity > LCD_TIMEOUT_MS)) {
        showIdleScreen();
    }

    AppConfig cfg = DataManager::getInstance().getConfig();
    if (xSemaphoreTake(DataManager::mutexI2C, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (cfg.system.lcd_enabled) {
            _lcd.backlight();
        } else {
            _lcd.noBacklight();
        }
        xSemaphoreGive(DataManager::mutexI2C);
    }
}

void Task_LCD(void *pvParameters) {
    LcdInterface::getInstance().begin();
    InputEvent ev;
    for (;;) {
        if (xQueueReceive(Hardware::inputQueue, &ev, pdMS_TO_TICKS(100)) == pdTRUE) {
            LcdInterface::getInstance().processInput(ev);
        }
        LcdInterface::getInstance().update();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
