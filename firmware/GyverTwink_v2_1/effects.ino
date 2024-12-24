void effects() {
  static Timer effTmr(30);
  static uint16_t countP = 0; // текущий кадр, зависящий от скорости
  static byte countSkip = 0;
  static byte prevEff = 255;
  static byte fadeCount = 0;

  if (effTmr.ready()) {
    byte thisPreset = curPreset;

    // эффект сменился
    if (prevEff != curPreset) {
      prevEff = curPreset;  
      fadeCount = 25;
    }

    byte scale = presets[thisPreset].scale;
    byte speed = presets[thisPreset].speed;
    byte curEffect = presets[thisPreset].effect_type;
    byte curPal = presets[thisPreset].palette;
    if (curPal >= ACTIVE_PALETTES) curPal -= ACTIVE_PALETTES;


    for (int i = 0; i < cfg.ledAm; i++) {
      CRGB color;

      switch (curEffect) {
        case 0: // сплошные линии
          color = stripesSt(i, countP, scale, curPal);
        break;
        case 1: // зебра-линии
          color = stripesStSec(i, countP, scale, curPal);
        break;
        case 2: // шум перлина
          color = perlinNoise(i, countP, scale, curPal);
        break;
        case 3: // сектора круга 1
          color = roundStripes(i, countP, scale, curPal, 0);
        break;
        case 4: // сектора круга 2
          color = roundStripes(i, countP, scale, curPal, 1);
        break;
        case 5: // сектора круга 3
          color = roundStripes(i, countP, scale, curPal, 2);
        break;
        case 6: // зебра сектора круга 1
          color = roundStripesSec(i, countP, scale, curPal, 0);
        break;
        case 7: // зебра сектора круга 1
          color = roundStripesSec(i, countP, scale, curPal, 1);
        break;
        case 8: // зебра сектора круга 1
          color = roundStripesSec(i, countP, scale, curPal, 2);
        break;
        case 9: // градиент из центра
          color = pulse(i, countP, scale, curPal);
        break;
        case 10: // блестки
          color = sparkles(i, countP, scale, curPal, speed);
        break;
        case 11: // зебра-линии и блестки
          color = stripesStSecSparkles(i, countP, scale, curPal);
        break;
        case 12: // прыгающие шарики
          color = JumpingBalls(i, scale, curPal);
        break;
        case 13: // метеориты со следом
          color = Meteors(i, scale, speed, curPal);
        break;
        default:
          color = CRGB::Red;
        break;
      }

      // плавная смена эффекта меняется за 25 фреймов
      if (fadeCount) leds[i] = blend(leds[i], color, 40);
      else leds[i] = color;
    }

    if (fadeCount) fadeCount--;

    countP += (speed - 128) / 10;
    strip->showLeds(cfg.bright);
  }
}


CRGB stripesSt(int i, uint16_t countP, byte scale, byte curPal) {
    byte idx;
    float angle = (float)scale / 128;
    float ss = angle*(1-angle)+angle; //аналог синуса
    float cs; // аналог косинуса
    if(angle < 1)
      cs = (angle + 1) * (1 - angle);
    else
      cs = (angle - 1) * (angle - 3);

    int y = (int)(xy[i][0]*ss + xy[i][1]*cs) * 5;// поворот, 5 - масштаб
    idx = y + countP;
    return ColorFromPalette(paletteArr[curPal], idx, 255, LINEARBLEND);
}


CRGB stripesStSec(int i, uint16_t countP, byte scale, byte curPal) {
    byte idx;
    float angle = (float)scale / 128;
    float ss = angle * (1 - angle) + angle;
    float cs;
    if(angle < 1)
      cs = (angle + 1) * (1 - angle);
    else
      cs = (angle - 1) * (angle - 3);

    byte y = (int)(xy[i][0]*ss + xy[i][1]*cs) * 5;
    idx = ((y + countP) / 70) * 70;
    if(((y + countP) / 70) % 2 == 0)
      return ColorFromPalette(paletteArr[curPal], idx, 255, LINEARBLEND);
    else 
      return CRGB::Black;
}


CRGB perlinNoise(int i, uint16_t countP, byte scale, byte curPal) {
    byte idx;
    idx = inoise8(xy[i][0] * scale / 10, xy[i][1] * scale / 10, countP);
    CRGB color = ColorFromPalette(paletteArr[curPal], idx, 255, LINEARBLEND);
    return color;
}


CRGB roundStripes(int i, uint16_t countP, byte scale, byte curPal, byte type) {
    byte idx = 0;
    byte c[2] = {0, 0};
    if (type == 0) { c[0] = mm.minX + mm.w/2; c[1] = mm.minY + mm.h/2; }
    if (type == 1) { c[0] = mm.minX; c[1] = mm.minY + 3*mm.h/4; }
    if (type == 2) { c[0] = mm.maxX; c[1] = mm.minY + 3*mm.h/4; }
    int deltaX = xy[i][0] - c[0]; //float
    int deltaY = xy[i][1] - c[1];
    float angle = atan2(deltaY, deltaX);

    int angleInDegrees = angle * 81.17;

    idx = angleInDegrees * ((float)scale/80) + countP;
    CRGB color = ColorFromPalette(paletteArr[curPal], idx, 255, LINEARBLEND);
    return color;
}


CRGB roundStripesSec(int i, uint16_t countP, byte scale, byte curPal, byte type) {
    byte idx = 0;
    byte c[2] = {0, 0};
    if (type == 0) { c[0] = mm.minX + mm.w/2; c[1] = mm.minY + mm.h/2; }
    if (type == 1) { c[0] = mm.minX; c[1] = mm.minY + 3*mm.h/4; }
    if (type == 2) { c[0] = mm.maxX; c[1] = mm.minY + 3*mm.h/4; }
    int deltaX = xy[i][0] - c[0];
    int deltaY = xy[i][1] - c[1];
    float angle = atan2(deltaY, deltaX);

    int angleInDegrees = angle * 81.17;

    idx = ((int)(angleInDegrees * ((float)scale/80)) + countP) / 70;

    if(idx % 2 == 0)
      return ColorFromPalette(paletteArr[curPal], idx*70, 255, LINEARBLEND);
    else 
      return CRGB::Black;
}


CRGB pulse(int i, uint16_t countP, byte scale, byte curPal) {
    byte idx = (sqrt(sq(xy[i][0] - mm.minX - mm.w/2) + sq(xy[i][1] - mm.minY - mm.h/2)) / 20 * scale) - countP;
    return ColorFromPalette(paletteArr[curPal], idx, 255, LINEARBLEND);
}


CRGB sparkles(int i, uint16_t countP, byte scale, byte curPal, int speed) {
    CRGB color = leds[i];
    if ((int)random(0, 512-scale*2) < 1) {
      return ColorFromPalette(paletteArr[curPal], random(0, 255), 255, LINEARBLEND);
    }
    else
      return color.fadeToBlackBy((2 + speed/3));
}


CRGB stripesStSecSparkles(int i, uint16_t countP, byte scale, byte curPal) {
    static int sparkles[LED_MAX/6];
    CRGB color = leds[i];
    byte idx;
    for(int j = 0; j < cfg.ledAm/6; j++) if(sparkles[j] == i) {
        if (color.getAverageLight() < 3) {
          sparkles[j] = random(0, cfg.ledAm-1);
          leds[sparkles[j]] = CRGB::White;
        }
        else
          return color.fadeToBlackBy(random(8, 12));
      }
    float angle = (float)scale / 128;
    float ss = angle * (1 - angle) + angle;
    float cs;
    if(angle < 1)
      cs = (angle+1) * (1 - angle);
    else
      cs = (angle-1) * (angle - 3);
    byte y = (int)(xy[i][0]*ss + xy[i][1]*cs) * 2;
    idx = ((y + countP) / 70) * 70;
    if(((y + countP) / 70) % 2 == 0)
      return ColorFromPalette(paletteArr[curPal], idx, 255, LINEARBLEND);
    else 
      return CRGB::Black;
}


CRGB JumpingBalls(int i, byte scale, byte curPal){
  static float b_pos[6][2];
  static float b_vel[6][2];
  static int b_col[6];
  static bool init_flag = false;
  if(!init_flag) {
    for(int j = 0; j < 6; j++){
      b_pos[j][0] = random(mm.minX, mm.maxX);
      b_pos[j][1] = random(mm.minY, mm.maxY);
      b_vel[j][0] = (float)random(-300, 300) / 100;
      b_vel[j][1] = (float)random(-300, 300) / 100;
      b_col[j] = random(0, 255);
    }
    init_flag = true;
  }

  if(i == 0) {
    for(int j = 0; j < scale/42; j++){
      b_pos[j][0] += b_vel[j][0];
      b_pos[j][1] += b_vel[j][1];
      if (b_pos[j][0] < mm.minX) { b_vel[j][0] = 0.9 * abs(b_vel[j][0]); b_pos[j][0] = mm.minX; }
      if (b_pos[j][0] > mm.maxX) { b_vel[j][0] = -0.9 * abs(b_vel[j][0]); b_pos[j][0] = mm.maxX; }
      if (b_pos[j][1] < mm.minY) { b_vel[j][1] = 0.94 * abs(b_vel[j][1]); b_pos[j][1] = mm.minY; }
      if (b_pos[j][1] > mm.maxY) { b_vel[j][1] = -0.94 * abs(b_vel[j][1]); b_pos[j][1] = mm.maxY; }
      b_vel[j][1] += 0.1;
      
      if (abs(b_vel[j][0]) + abs(b_vel[j][1]) < 1 && abs(b_pos[j][1]) > mm.maxY-5) {
        b_vel[j][0] = (float)random(-300, 300) / 100;
        b_vel[j][1] = (float)random(0, 500) / 100;
      }
    }
  }
  
  for(int j = 0; j< scale/42; j++) {
    if(sq(xy[i][0] - b_pos[j][0]) + sq(xy[i][1] - b_pos[j][1]) < 80)
        return ColorFromPalette(paletteArr[curPal], b_col[j], 255, LINEARBLEND);
  }
  return CRGB::Black;
}


CRGB Meteors(int i, byte scale, byte speed, byte curPal){
  static float vel[2];
  static float b_pos[6][2];
  static int b_col[6];
  static bool init_flag = false;
  
  if(!init_flag) {
    for(int j = 0; j<6; j++) {
      vel[0] = random(-2, 2);
      vel[1] = 1;
      b_pos[j][0] = random(mm.minX, mm.maxX);
      b_pos[j][1] = random(mm.minY, mm.maxY);
      b_col[j] = random(0, 255);
    }
    init_flag = true;
  }
  if(i == 0) {
    for(int j = 0; j<scale/42; j++){
      b_pos[j][0] += vel[0];
      b_pos[j][1] += vel[1];
      if((int)random(0, 255) < 2)
       b_pos[j][1] += 1;
      if (b_pos[j][0] < mm.minX) { b_pos[j][0] = mm.maxX; }
      if (b_pos[j][0] > mm.maxX) { b_pos[j][0] = mm.minX; }
      if (b_pos[j][1] < mm.minY) { b_pos[j][1] = mm.maxY; }
      if (b_pos[j][1] > mm.maxY) { b_pos[j][1] = mm.minY; }
    }
  }
  
  for(int j = 0; j < scale/42; j++) {
    if(sq(xy[i][0] - b_pos[j][0]) + sq(xy[i][1] - b_pos[j][1]) < 30)
        return ColorFromPalette(paletteArr[curPal], b_col[j], 255, LINEARBLEND);
  }
  return leds[i].fadeToBlackBy(5+(speed/5));
}