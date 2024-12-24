#define MAX_UDP_PACKET 30
byte ubuf[MAX_UDP_PACKET];

void reply(byte* data, byte size) {
  udp.beginPacket(udp.remoteIP(), udp.remotePort());
  udp.write("GT");
  udp.write(data, size);
  udp.endPacket();
}

void parsing() {
  static uint32_t tmr;
  if (udp.parsePacket()) {
    if (millis() - tmr < 50) {  // "антидребезг" приёма
      udp.read(ubuf, MAX_UDP_PACKET);
      return;
    }
    tmr = millis();
    int n = udp.read(ubuf, MAX_UDP_PACKET);
    ubuf[n] = 0;
    DEBUG("received: G T ");
    if (ubuf[0] != 'G' || ubuf[1] != 'T') return;
    for (int i = 2; i < n; i++) {
      DEBUG(ubuf[i]);
      DEBUG(" ");
      }
      DEBUGLN();
    byte answ[11];

    switch (ubuf[2]) {
      case 0:   // запрос IP
        delay(myIP[3] * 2);
        answ[0] = 0;
        answ[1] = myIP[3];
        reply(answ, 2);
        break;

      case 1:   // запрос настроек
      {
        answ[0] = 1;
        answ[1] = 0;
        answ[2] = cfg.ledAm / 100;
        answ[3] = cfg.ledAm % 100;
        answ[4] = cfg.power;
        answ[5] = cfg.bright;
        answ[6] = cfg.autoCh;
        answ[7] = cfg.rndCh;
        answ[8] = cfg.prdCh;
        answ[9] = cfg.turnOff;
        answ[10] = cfg.offTmr;
        reply(answ, 11); //отправка настроек

        //разбиение и отправка координат светодиодов
        byte answBig[60*2+2]; 
        answBig[0] = 1;

        int dataChunk = 2;
        int i = 0;
        for (; (i+1)*60 < 200; i++) {
          answBig[1] = i+1;
          for (int j = 0; j < 60; j++){
            answBig[j*2+2] = xy[i*60+j][0];
            answBig[j*2+1+2] = xy[i*60+j][1];
          }
          reply(answBig, 60*2+2); //отправка координат светодиодов
        }

        answBig[1] = i+1;
        for (int j = 0; j < 200-i*60; j++){
          answBig[j*2+2] = xy[i*60+j][0];
          answBig[j*2+1+2] = xy[i*60+j][1];
        }
        reply(answBig, 2*(200-i*60)+2); //отправка координат светодиодов
      }
      break;

      case 2:   // приём настроек
        drawF = false;
        switch (ubuf[3]) {
          case 0: cfg.ledAm = ubuf[4] * 100 + ubuf[5];
            strip->setLeds(leds, cfg.ledAm);
            break;
          case 1: cfg.power = ubuf[4];
            break;
          case 2: cfg.bright = ubuf[4];
            break;
          case 3: cfg.autoCh = ubuf[4];
            if (cfg.autoCh) switchTmr.restart();
            else switchTmr.stop();
            break;
          case 4: cfg.rndCh = ubuf[4];
            break;
          case 5: cfg.prdCh = ubuf[4];
            switchTmr.setPrd(cfg.prdCh * 60000ul);
            if (cfg.autoCh) switchTmr.restart();
            break;
          case 6:   // нехт эффект
            switchEff();
            if (cfg.autoCh) switchTmr.restart();
            return;
            break;
          case 7:
            cfg.turnOff = ubuf[4];
            if (cfg.turnOff) offTmr.restart();
            else offTmr.stop();
            break;
          case 8:
            cfg.offTmr = ubuf[4];
            offTmr.setPrd(cfg.offTmr * 60000ul);
            if (cfg.turnOff) offTmr.restart();
            break;
        }
        if (!cfg.power) strip->showLeds(0);
        EEcfg.update();
        break;

      case 3:
        switch (ubuf[3]) {
          case 0:   // запуск калибровки
            DEBUGLN("Calibration start");
            calibF = true;
            strip->clearLedData();
            strip->showLeds(0);
            break;

          case 1:   // следующий лед
            {
              int curLed = ubuf[4] * 100 + ubuf[5];
              if (curLed > 0) {
                xy[curLed - 1][0] = ubuf[6];
                xy[curLed - 1][1] = ubuf[7];
              }
              strip->clearLedData();
              leds[curLed] = CRGB::White;
              strip->showLeds(255);
            }
            break;

          case 2:   // калибровка окончена
            DEBUGLN("Calib Finished");
            calibF = false;
            strip->clearLedData();
            strip->showLeds(0);
            EExy.updateNow();
            calculateMinMax(); // Вычисление мин/макс значений
            
            DEBUG("After calib: ");
            printLedPos();
            EEmm.updateNow();
            break;
            
          case 3:   // v2 выкл светодиоды
            {
              strip->clearLedData();
              strip->showLeds(255);
            }
            break;

          case 4:   // v2 вкл n-ый светодиод
            {
              int curLed = ubuf[4] * 100 + ubuf[5];
              strip->clearLedData();
              leds[curLed] = CRGB::White;
              strip->showLeds(255);
            }
            break;
        }
        break;

      case 4:   // управление пресетами
        drawF = false;
        EEpsets.update();
        switch (ubuf[3]) {
          case 0:   // выбор пресета в дропе
            curPreset = ubuf[4];
            answ[0] = 4;
            answ[1] = presets[curPreset].fav;
            answ[2] = presets[curPreset].scale;
            answ[3] = presets[curPreset].speed;
            answ[4] = presets[curPreset].effect_type;
            answ[5] = presets[curPreset].palette;
            reply(answ, 6);
            break;
          case 1:   // флажок избранное
            presets[curPreset].fav = ubuf[4];
            break;
          case 2:   // масштаб
            presets[curPreset].scale = ubuf[4];
            break;
          case 3:   // скорость
            presets[curPreset].speed = ubuf[4];
            break;
          case 4:   // тип эффекта
            presets[curPreset].effect_type = ubuf[4];
            break;
          case 5:   // палитра
            presets[curPreset].palette = ubuf[4];
            break;
        }
        break;

      case 5:
        switch (ubuf[3]) {
          case 0:
            drawF = true;
            strip->clearLedData();
            strip->showLeds(255);
          break;
          case 1:
          {
            int curLed = ubuf[4] * 100 + ubuf[5];

            leds[curLed] = CRGB(ubuf[6], ubuf[7], ubuf[8]);
            strip->showLeds(255);
          }
          break;
          case 2:
            drawF = false;
          break;
        }
      break;
    }
  }
}

#define MAX_VALUES 5    // Количество значений для вычисления среднего
void calculateMinMax() {
  int minXValues[MAX_VALUES] = {255, 255, 255, 255, 255};
  int maxXValues[MAX_VALUES] = {0, 0, 0, 0, 0};
  int minYValues[MAX_VALUES] = {255, 255, 255, 255, 255};
  int maxYValues[MAX_VALUES] = {0, 0, 0, 0, 0};

  for (int i = 0; i < cfg.ledAm; i++) {
    for (int j = 0; j < MAX_VALUES; j++) {
        // Обновление массива минимальных значений X
        if (xy[i][0] <= minXValues[j]) {
            for (int k = MAX_VALUES - 1; k > j; k--) {
                minXValues[k] = minXValues[k - 1]; // Сдвиг значений
            }
            minXValues[j] = xy[i][0]; // Вставка нового минимума
            break;
        }
    }
    for (int j = 0; j < MAX_VALUES; j++) {
        // Обновление массивов максимальных значений X
        if (xy[i][0] >= maxXValues[j]) {
            for (int k = MAX_VALUES - 1; k > j; k--) {
                maxXValues[k] = maxXValues[k - 1]; // Сдвиг значений
            }
            maxXValues[j] = xy[i][0]; // Вставка нового максимума
            break;
        }
    }
    for (int j = 0; j < MAX_VALUES; j++) {
        if (xy[i][1] <= minYValues[j]) {
            for (int k = MAX_VALUES - 1; k > j; k--) {
                minYValues[k] = minYValues[k - 1];
            }
            minYValues[j] = xy[i][1];
            break;
        }
    }
    for (int j = 0; j < MAX_VALUES; j++) {
        if (xy[i][1] >= maxYValues[j]) {
            for (int k = MAX_VALUES - 1; k > j; k--) {
                maxYValues[k] = maxYValues[k - 1];
            }
            maxYValues[j] = xy[i][1];
            break;
        }
      }
    }

  int minX = 0;
  int maxX = 0;
  int minY = 0;
  int maxY = 0;

  // Вычисление среднего для минимальных значений
  for (int j = 0; j < MAX_VALUES; j++) {
      minX += minXValues[j];
      minY += minYValues[j];
      maxX += maxXValues[j];
      maxY += maxYValues[j];

      DEBUG(minXValues[j]);
      DEBUG(" ");
      DEBUG(minYValues[j]);
      DEBUG(" ");
      DEBUG(maxXValues[j]);
      DEBUG(" ");
      DEBUG(maxYValues[j]);
      DEBUGLN(" ");
  }
  minX /= MAX_VALUES;
  minY /= MAX_VALUES;
  maxX /= MAX_VALUES;
  maxY /= MAX_VALUES;

  mm.minX = minX;
  mm.minY = minY;
  mm.maxX = maxX;
  mm.maxY = maxY;
  mm.w = mm.maxX - mm.minX;
  mm.h = mm.maxY - mm.minY;

  DEBUG("min: (");
  DEBUG(mm.minX);
  DEBUG("; ");
  DEBUG(mm.minY);
  DEBUG(") max: (");
  DEBUG(mm.maxX);
  DEBUG("; ");
  DEBUG(mm.maxY);
  DEBUG(") w/h: (");
  DEBUG(mm.w);
  DEBUG("; ");
  DEBUG(mm.h);
  DEBUGLN(")");
}
