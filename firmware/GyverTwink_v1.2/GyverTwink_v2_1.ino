/*
  Скетч к проекту "GyverTwink"
  - Страница проекта (схемы, описания): https://alexgyver.ru/gyvertwink/
  - Исходники на GitHub: https://github.com/AlexGyver/GyverTwink
  Проблемы с загрузкой? Читай гайд для новичков: https://alexgyver.ru/arduino-first/
  AlexGyver, AlexGyver Technologies, 2021

  Пины в прошивке указаны для Wemos (D-пины), для загрузки выбирать плату ESP8266 boards/LOLIN Wemos D1 & R2 mini
  Рекомендуется использовать последнюю 2.x версию ядра esp8266 (я использую 2.7.4), потому что на новой версии (3.x) индусы опять что то сломали и на ленте постоянно мигает первый светодиод
  По умолчанию в прошивке настроено (блок настроек в самом начале кода программы):
  Лента 2812. Если нужна другая лента (2811, 2813) — исправляем в настройках в самом начале кода прошивки.
  Порядок цветов — GRB. Для 2811 ленты вроде бы нужно поменять на GBR
  Максимальная длина — 500 светодиодов. Можно сделать больше.
  Также в прошивке можно изменить имя точки для работы Access Point
  При использовании платы esp8266 из другого проекта (с другой прошивкой) рекомендуется выбрать в настройках Инструменты/Erase Flash/All Flash Contents
  Если хотите использовать гирлянду из пикселей (с колпачками) — она питается от 12V. Схема есть выше.
  При использовании пиксельной гирлянды на чипах WS2811 или UCS1903 нужно указать модель чипа WS2811 в прошивке!


            ВАЖНО!!!     Версия платы esp8266 - 2.7.4      ВАЖНО!!!

               Версия fastLed - 3.9.7 (по идее не должно влиять)
*/

/*
  1.1 - исправлена калибровка больше 255 светодиодов
  1.2 - исправлена ошибка с калибровкой
  2.0 - добавлены эффекты, улучшена калибровка, добавлено функция рисования

  2.1 - добавлены разные виды эффектов, изменена система сохранения
  теперь все храниться в виде пресетов, в котором записаны настройки, палитра и тип эффекта
  всего 15 пресетов, но это можно поменять в прошивке и в коде приложения
  - При калибровке через приложении надо 
  1.закреить телефон
  2. расположить телефон так, чтобы елка помещалась в большую часть кадра
  если программа не может найти светодиод по камере, она выставит ему позицию прошлого, но x+1 
*/


/*
  Мигает синим - открыт портал
  Мигает жёлтым - подключаемся к точке
  Мигнул зелёным - подключился к точке
  Мигнул красным - ошибка подключения к точке
  Мигнул розовым - создал точку
*/

// ================ НАСТРОЙКИ ================
#define BTN_PIN D3      // пин кнопки
#define BTN_TOUCH 0     // 1 - сенсорная кнопка, 0 - нет

#define LED_PIN D1      // пин ленты
#define LED_TYPE WS2812B // чип ленты
#define LED_ORDER GRB   // порядок цветов ленты
#define LED_MAX 500     // макс. светодиодов

#define PRESETS_SIZE 15 //число предустановок (связано с processing)
#define ACTIVE_PALETTES 64 // число палитр (связано с processing)

// имя точки в режиме AP
#define GT_AP_SSID "GyverTwink"
#define GT_AP_PASS "18273645"
//#define DEBUG_SERIAL_GT   // раскомментируй, чтобы включить отладку

// ================== LIBS ==================
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SimplePortal.h>
#include <FastLED.h>
#include <EEManager.h>
#include <EncButton.h>
#include "palettes.h"
#include "Timer.h"

// ================== OBJECTS ==================
WiFiServer server(80);
WiFiUDP udp;
EEManager EEwifi(portalCfg);
CRGB leds[LED_MAX];
CLEDController *strip;
EncButton<EB_TICK, BTN_PIN> btn;
IPAddress myIP;

// ================== EEPROM BLOCKS ==================
struct Cfg {
  uint16_t ledAm = 50;
  bool power = 1;
  byte bright = 100;
  bool autoCh = 0;
  bool rndCh = 0;
  byte prdCh = 1;
  bool turnOff = 0;
  byte offTmr = 60;
};
Cfg cfg;
EEManager EEcfg(cfg);

byte xy[LED_MAX][2];
EEManager EExy(xy);

struct MM {
  byte minY = 108;
  byte maxY = 158;
  byte minX = 128;
  byte maxX = 158;
  byte w = 30;
  byte h = 30;
};
MM mm;
EEManager EEmm(mm);

struct Presets {
  bool fav = true;
  byte scale = 50;
  byte speed = 150;
  byte effect_type = 0;
  byte palette = 0;
};
Presets presets[PRESETS_SIZE];
EEManager EEpsets(presets);

// ================== MISC DATA ==================
Timer switchTmr(0, false);
Timer offTmr(60000, false);
bool calibF = false;
bool drawF = false;
byte curPreset = 0; // текущий пресет

#ifdef DEBUG_SERIAL_GT
#define DEBUGLN(x) Serial.println(x)
#define DEBUG(x) Serial.print(x)
#else
#define DEBUGLN(x)
#define DEBUG(x)
#endif

// ================== SETUP ==================
void setup() {
#ifdef DEBUG_SERIAL_GT
  Serial.begin(115200);
  DEBUGLN();
#endif
  delay(200);
  if (BTN_TOUCH) btn.setButtonLevel(HIGH);
  startStrip();
  EEPROM.begin(2048); // с запасом!

  // если это первый запуск или щелчок по кнопке, открываем портал
  if (EEwifi.begin(0, 'a') || checkButton()) portalRoutine();

  // создаём точку или подключаемся к AP
  if (portalCfg.mode == WIFI_AP || (portalCfg.mode == WIFI_STA && portalCfg.SSID[0] == '\0')) setupAP();
  else setupSTA();
  DEBUGLN(myIP);

  EEcfg.begin(EEwifi.nextAddr(), 'a');
  EEpsets.begin(EEcfg.nextAddr(), 'a');
  EEmm.begin(EEpsets.nextAddr(), (uint8_t)LED_MAX);
  EExy.begin(EEmm.nextAddr(), (uint8_t)LED_MAX);

  switchTmr.setPrd(cfg.prdCh * 60000ul);
  if (cfg.autoCh) switchTmr.restart();
  switchEff();
  cfg.turnOff = false;
  strip->setLeds(leds, cfg.ledAm);
  udp.begin(8888);
}

// ================== LOOP ==================
void loop() {
  button();   // опрос кнопки

  // менеджер епром
  EEcfg.tick();
  EEpsets.tick();

  parsing();  // парсим udp

  // форс выключен и настало время менять эффект
  if (switchTmr.ready()) switchEff();

  // таймер выключения
  if (offTmr.ready()) {
    offTmr.stop();
    cfg.turnOff = false;
    cfg.power = false;
    strip->showLeds(0);
    EEcfg.update();
    DEBUGLN("Off tmr");
  }

  // показываем эффект, если включены
  if (!calibF && cfg.power && !drawF) effects();
}

void printLedPos() {
  Serial.println("Leds Positions");
  for (int i = 0; i < cfg.ledAm; i++){
    Serial.print(i);
    Serial.print(": ");
    Serial.print(xy[i][0]);
    Serial.print(" ");
    Serial.println(xy[i][1]);
  }
  Serial.println("end Leds Positions");
}
