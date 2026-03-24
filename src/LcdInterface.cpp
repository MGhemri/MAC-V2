#include "LcdInterface.h"
#include "DataManager.h"
#include "Hardware.h"
#include <WiFi.h>

// Action callbacks
void actionToggleRelay1() { Hardware::getInstance().toggleRelay(1); }
void actionToggleRelay2() { Hardware::getInstance().toggleRelay(2); }
void actionToggleRelay3() { Hardware::getInstance().toggleRelay(3); }
void actionToggleRelay4() { Hardware::getInstance().toggleRelay(4); }
void actionToggleRelay5() { Hardware::getInstance().toggleRelay(5); }
void actionToggleRelay6() { Hardware::getInstance().toggleRelay(6); }
void actionToggleRelay7() { Hardware::getInstance().toggleRelay(7); }
void actionToggleRelay8() { Hardware::getInstance().toggleRelay(8); }
void actionReboot() { ESP.restart(); }

// Correct LcdMenu v3.x syntax: ItemHeader and ItemFooter might not be separate files or needed.
// Based on the file list, ItemCommand and ItemSubMenu exist.
MenuItem* mainMenu[] = {
    new ItemCommand("Relay 1", actionToggleRelay1),
    new ItemCommand("Relay 2", actionToggleRelay2),
    new ItemCommand("Relay 3", actionToggleRelay3),
    new ItemCommand("Relay 4", actionToggleRelay4),
    new ItemCommand("Relay 5", actionToggleRelay5),
    new ItemCommand("Relay 6", actionToggleRelay6),
    new ItemCommand("Relay 7", actionToggleRelay7),
    new ItemCommand("Relay 8", actionToggleRelay8),
    new ItemCommand("Reboot", actionReboot)
};

LcdMenu menu(LCD_ROWS, LCD_COLS);

LcdInterface::LcdInterface() : _lcd(LCD_ADDR, LCD_COLS, LCD_ROWS), _lastActivity(0), _isIdle(true), _isShowingIP(false) {}

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
    // Library v3.5.x setup
    menu.setupLcdWithMenu(0x27, mainMenu);
    showIdleScreen();
}

void LcdInterface::showIdleScreen() {
    if (xSemaphoreTake(DataManager::mutexI2C, pdMS_TO_TICKS(1000)) == pdTRUE) {
        _lcd.clear();
        _lcd.setCursor(0, 0);
        _lcd.print("MAC-V2 Controller");
        _lcd.setCursor(0, 1);
        if (WiFi.getMode() == WIFI_AP) {
            _lcd.print("AP: MAC-V2-CONFIG");
        } else {
            _lcd.print("IP: ");
            _lcd.print(WiFi.localIP());
        }

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
    _isShowingIP = false;
}

void LcdInterface::showConnectionInfo() {
    if (xSemaphoreTake(DataManager::mutexI2C, pdMS_TO_TICKS(1000)) == pdTRUE) {
        _lcd.clear();
        _lcd.setCursor(0, 0);
        _lcd.print("WiFi Connecte !");
        _lcd.setCursor(0, 1);
        _lcd.print("IP: ");
        _lcd.print(WiFi.localIP());
        _lcd.setCursor(0, 3);
        _lcd.print("Cliquez pour menu");
        xSemaphoreGive(DataManager::mutexI2C);
    }
    _isShowingIP = true;
    _isIdle = false;
}

void LcdInterface::processInput(InputEvent event) {
    _lastActivity = millis();

    if (_isShowingIP) {
        if (event == InputEvent::CLICK) {
            showIdleScreen();
        }
        return;
    }

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
