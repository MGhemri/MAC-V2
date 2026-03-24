#include "LcdInterface.h"
#include "DataManager.h"
#include "Hardware.h"
#include "ApiServer.h"
#include <WiFi.h>

// Forward declarations
void actionToggleRelay1() { Hardware::getInstance().toggleRelay(1); LcdInterface::getInstance().updateMenuLabels(); }
void actionToggleRelay2() { Hardware::getInstance().toggleRelay(2); LcdInterface::getInstance().updateMenuLabels(); }
void actionToggleRelay3() { Hardware::getInstance().toggleRelay(3); LcdInterface::getInstance().updateMenuLabels(); }
void actionToggleRelay4() { Hardware::getInstance().toggleRelay(4); LcdInterface::getInstance().updateMenuLabels(); }
void actionToggleRelay5() { Hardware::getInstance().toggleRelay(5); LcdInterface::getInstance().updateMenuLabels(); }
void actionToggleRelay6() { Hardware::getInstance().toggleRelay(6); LcdInterface::getInstance().updateMenuLabels(); }
void actionToggleRelay7() { Hardware::getInstance().toggleRelay(7); LcdInterface::getInstance().updateMenuLabels(); }
void actionToggleRelay8() { Hardware::getInstance().toggleRelay(8); LcdInterface::getInstance().updateMenuLabels(); }

void actionStartWiFi();
void actionReboot() { ESP.restart(); }
void actionBack();

// Need to declare pointers for dynamic updates
MenuItem* rItems[9];

// Define menus without macros for more control
MenuItem* relay_submenu[] = {
    new ItemHeader(), // Placeholder for parent
    new ItemCommand("R1: Initializing", actionToggleRelay1),
    new ItemCommand("R2: Initializing", actionToggleRelay2),
    new ItemCommand("R3: Initializing", actionToggleRelay3),
    new ItemCommand("R4: Initializing", actionToggleRelay4),
    new ItemCommand("R5: Initializing", actionToggleRelay5),
    new ItemCommand("R6: Initializing", actionToggleRelay6),
    new ItemCommand("R7: Initializing", actionToggleRelay7),
    new ItemCommand("R8: Initializing", actionToggleRelay8),
    new ItemCommand("<- Retour", actionBack),
    new ItemFooter()
};

MenuItem* mainMenu[] = {
    new ItemHeader(),
    new ItemSubMenu("1. Controle Relais", relay_submenu),
    new ItemCommand("2. Config Wi-Fi", actionStartWiFi),
    new ItemCommand("3. Redemarrer", actionReboot),
    new ItemFooter()
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
    menu.setupLcdWithMenu(0x27, mainMenu);
    showIdleScreen();
}

void LcdInterface::updateMenuLabels() {
    AppConfig cfg = DataManager::getInstance().getConfig();
    for (int i = 0; i < 8; i++) {
        char buffer[21];
        String name = (cfg.relays[i].mode == "custom") ? cfg.relays[i].custom_name : cfg.relays[i].mode;
        if (name == "") name = "Relay " + String(i+1);
        String state = cfg.relays[i].state ? "[ON]" : "[OFF]";
        snprintf(buffer, sizeof(buffer), "R%d:%-8s %4s", i+1, name.substring(0,8).c_str(), state.c_str());
        relay_submenu[i+1]->setText(strdup(buffer));
    }
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
        _lcd.print("Relais actifs: ");
        _lcd.print(active);
        _lcd.print("/8");

        _lcd.setCursor(0, 3);
        _lcd.print("Clic pour Menu");
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
            updateMenuLabels();
            if (xSemaphoreTake(DataManager::mutexI2C, pdMS_TO_TICKS(1000)) == pdTRUE) {
                _lcd.clear();
                xSemaphoreGive(DataManager::mutexI2C);
                menu.show();
            }
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

void actionStartWiFi() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MAC-V2-CONFIG");
    LcdInterface::getInstance().showConnectionInfo(); // Reuse info screen to show AP
}

void actionBack() {
    menu.back();
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
