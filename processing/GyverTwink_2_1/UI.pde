byte curTab = 0;
TextInput leds = new TextInput();
TextInput subnet = new TextInput();
DropDown dropIP = new DropDown();
DropDown dropPalt1 = new DropDown();
DropDown dropPalt2 = new DropDown();
DropDown dropPalt3 = new DropDown();
DropDown dropEff = new DropDown();
Toggle power = new Toggle();
Toggle offT = new Toggle();
Toggle auto = new Toggle();
Toggle rnd = new Toggle();
Slider bri = new Slider();
Slider prd = new Slider();
Slider offS = new Slider();
Toggle fav = new Toggle();
Slider scl = new Slider();
Slider spd = new Slider();

Slider hue = new Slider();
Slider sat = new Slider();
Slider brn = new Slider();

int current_effs_tab = 0;
//private static final byte palettes_amount = 64;
private static final byte presets_amount = 15;

String[] effs = {
  "solid lines",
  "split lines",
  "perlin noise",
  //"arrows",
  "circle centre",
  //"circle_up",
  "circle left",
  "circle right",
  "circle stripes centre",
  "circle stripes left",
  "circle stripes right",
  "pulse",
  "sparkles",
  "split lines + sparkles",
  "jumping balls",
  "meteors"
};

String[] palt1 = { //23
  "HeatColors_p",
  "Fire_gp",
  "LithiumFireColors_p",
  "SodiumFireColors_p",
  "CopperFireColors_p",
  "AlcoholFireColors_p",
  "LavaColors_p",
  "PartyColors_p",
  "RainbowColors_p",
  "RainbowStripeColors_p",
  "OceanColors_p",
  "ForestColors_p",
  "Sunset_Real_gp",
  "dkbluered_gp",
  "Optimus_Prime_gp",
  "warmGrad_gp",
  "coldGrad_gp",
  "hotGrad_gp",
  "pinkGrad_gp",
  "comfy_gp",
  "cyperpunk_gp",
  "girl_gp",
  "xmas_gp"
};

String[] palt2 = { //22
  "acid_gp",
  "gummy_gp",
  "leo_gp",
  "aurora_gp",
  "redwhite_gp",
  //--------------new-----------
  "bhw1_28_gp",
  "bhw2_whooo_gp",
  "bhw2_thanks_gp",
  "eawc_sub_gp",
  "paintjet_gp",
  "calbayo_21_gp",
  "shikon_21_gp",
  "purple_orange_d05_gp",
  "cbcPiYG_gp",
  "PuOr_gp",
  "Need_I_Say_More_gp",
  "Alive_And_Kicking_gp",
  "icing_on_the_cake_gp",
  "mellon_ball_surprise_gp",
  "Pastels_gp",
  "Analogous_06_gp",
  "Split_Complementary_06_gp"
};
String[] palt3 = { //19
  "Split_04_gp",
  "Split_Complementary_07_gp",
  "Split_Complementary_03a_gp",
  "Complementary_06a_gp",
  "rainbowsherbet_gp",
  "pastel_rb3_gp",
  "beading_gp",
  "dances_with_fairies_gp",
  "tranquil_gp",
  "springfloralblend_gp",
  "tropical_beach_gp",
  "heartbalance_gp",
  "omageerte3_gp",
  "qual_dark_06_gp",
  "tashangel_gp",
  "renmaiden_gp",
  "trove_gp",
  "zebra_gp",
  "heat_gp"
};

void ui() {
  uiFill();
  // ====== TABS =======
  int w = width / 4;
  int h = w / 2;
  int y = height - h;

  if (IconButton("wrench", 0, y, w, h, curTab == 0)) switchCfg();
  if (IconButton("adjust", w*1, y, w, h, curTab == 1)) switchEffects();
  if (IconButton("camera", w*2, y, w, h, curTab == 2)) switchCalib();
  if (IconButton("draw", w*3, y, w, h, curTab == 3)) switchDrw();

  if (curTab == 0) cfgTab();
  if (curTab == 1) effTab();
  if (curTab == 2) calibTab();
  if (curTab == 3) drawTab();
}

void cfgTab() {
  uiGlobalX(offs);
  uiResetStep(20);
  LabelCenter("GyverTwink", 20);
  Divider(width-offs*2);

  Label("Subnet:", 15);
  Label("Connection:", 15);
  if (found) {
    Divider(width-offs*2);
    Label("LED amount:", 15);
    Label("Power:", 15);
    Label("Brightness:", 15);
    Divider(width-offs*2);
    Label("Off timer:", 15);
    Label("Turn off in [1-240m]:", 15);
    Divider(width-offs*2);
    Label("Switch effect:", 15);
    Label("Auto:", 15);
    Label("Random:", 15);
    Label("Period [1-10m]:", 15);
  }

  uiResetStep(20);
  uiStep();
  uiStep();

  if (found) {
    uiStep();
    uiStep();
    uiStep();
    if (leds.show(WW, uiStep(), W) && androidMode) openKeyboard();
    if (leds.done()) {
      if (androidMode) closeKeyboard();
      int am = int(leds.text);
      sendData(new int[] {2, 0, am/100, am % 100});
    }
    if (power.show(WW, uiStep())) sendData(new int[] {2, 1, int(power.value)});
    if (bri.show(0, 255, WW, uiStep(), W)) sendData(new int[] {2, 2, int(bri.value)});
    uiStep();
    if (offT.show(WW, uiStep())) sendData(new int[] {2, 7, int(offT.value)});
    if (offS.show(0, 250, WW, uiStep(), W)) sendData(new int[] {2, 8, int(offS.value)});
    uiStep();
    if (Button("Next effect", WW, uiStep(), W)) sendData(new int[] {2, 6});
    if (auto.show(WW, uiStep())) sendData(new int[] {2, 3, int(auto.value)});
    if (rnd.show(WW, uiStep())) sendData(new int[] {2, 4, int(rnd.value)});
    if (prd.show(1, 10, WW, uiStep(), W)) sendData(new int[] {2, 5, int(prd.value)});
  }

  uiResetStep(20);
  uiStep();
  uiStep();
  if (subnet.show(WW, uiStep(), W) && androidMode) openKeyboard();
  if (subnet.done()) {
    if (androidMode) closeKeyboard();
    file[0] = subnet.text;
    saveStrings("subnet.txt", file);
  }
  if (dropIP.show(ips.array(), WW, uiStep(), W-s_height)) {
    curIP = ips.get(dropIP.getSelected());
    requestCfg();
  }
  if (IconButton("sync", WW + W-s_height, uiPrevStep())) startSearch();
}


void effTab() {
  uiGlobalX(offs);
  uiResetStep(50);
  uiGlobalX(offs);
  if (found) {
    if (androidMode) uiSetScale(androidScale*0.8);
    else uiSetScale(pcScale*0.7);
    
    if(Button("<--", 15) && curPreset > 0){
      sendData(new int[] {4, 0, --curPreset});
      parseMode = 4;
    }
    uiResetStep(50);
    if(Button("-->", width-115) && curPreset < presets_amount-1) {
      sendData(new int[] {4, 0, ++curPreset});
      parseMode = 4;
    }
    Label("Palette: " + getPaletteName(curPalette), 20);
    uiStep();
    uiStep();
    Label("Effect: " + getEffectName(curEffect), 20);
    Label("Favorite:", 20);
    Label("Scale:", 20);
    Label("Speed:", 20);

    uiResetStep(50);
    Label("Preset " + (curPreset + 1), 25, width/2-50);
    uiStep();
    uiStep();
    uiStep();
    uiStep();
    if (fav.show(WW, uiStep())) sendData(new int[] {4, 1, int(fav.value)});
    if (scl.show(0, 255, WW, uiStep(), W)) sendData(new int[] {4, 2, int(scl.value)});
    if (spd.show(0, 255, WW, uiStep(), W)) sendData(new int[] {4, 3, int(spd.value/2+64)});

    uiResetStep(50);
    uiStep();
    if (dropPalt1.show(palt1, WW, uiStep(), W-s_height)) {
      sendData(new int[] {4, 5, dropPalt1.selected});
      curPalette = getNewPalette();
      current_effs_tab = 0;
    }
    if (!dropPalt1.getOpen() && dropPalt2.show(palt2, WW, uiStep(), W-s_height)) {
      sendData(new int[] {4, 5, palt1.length + dropPalt2.selected});
      curPalette = getNewPalette();
      current_effs_tab = 1;
    }
    if (!dropPalt1.getOpen() && !dropPalt2.getOpen() && dropPalt3.show(palt3, WW, uiStep(), W-s_height)) {
      sendData(new int[] {4, 5, palt1.length + palt2.length + dropPalt3.selected});
      curPalette = getNewPalette();
      current_effs_tab = 2;
    }
    
    if (!dropPalt1.getOpen() && !dropPalt2.getOpen() && !dropPalt3.getOpen() && dropEff.show(effs, WW, uiStep(), W-s_height)) {
      sendData(new int[] {4, 4, dropEff.selected});
      curEffect = dropEff.selected;
    }
    
    
    if (dropPalt1.getOpen()) { dropPalt2.setOpen(false); dropPalt3.setOpen(false); dropEff.setOpen(false);}
    if (dropPalt2.getOpen()) { dropPalt1.setOpen(false); dropPalt3.setOpen(false); dropEff.setOpen(false);}
    if (dropPalt3.getOpen()) { dropPalt1.setOpen(false); dropPalt2.setOpen(false); dropEff.setOpen(false);}
    
    if (androidMode) uiSetScale(androidScale);
    else uiSetScale(pcScale);
  } else Label("No devices detected!", 15);
}

void calibTab() {
  if (found) {
    // Камера не стартовала в PC режиме
    if (!androidMode && Wcam == null) return;

    if (camReady) { //обновлеие кадра по готовости камеры
      camReady = false;
      readCam();
      makeMap(1);
      findMax();
    }
    PImage frameScaled = (PImage)drawMap(3);
    image(frameScaled, (width-frameScaled.width)/2, 0);
    //PImage frameScaled = frame.copy();
    //frameScaled.resize(0, height*4/5);
    //image(frameScaled, (width-frameScaled.width)/2, 0);
    //if (calibF) {
    frameScaled = ring.copy();
    //frameScaled.resize(0, height*4/5);
    image(frameScaled, (width-frameScaled.width)/2, 0);
    //}

    //image(frame, (width-frame.width)/2, 0);
    //if (calibF) image(ring, (width-ring.width)/2, 0);

    uiResetStep(height - width/6 - 2*_step_y);
    uiResetX(0);
    uiGlobalX(0);

    if (Button("Start")) {
      calibF = true;
      sendData(new int[] {3, 0});
      calibCount = 0;
      actionTmr = millis() + 1000;
      ledPos = new ArrayList<int[]>();
      drawColor = new IntList();
    }

    Label(str(calibCount*100/(int(leds.text)+1))+'%', 15, uiPrevX()+15, uiPrevStep());
    if (Button("Stop")) {
      calibF = false;
      sendData(new int[] {3, 2});
      calibCount = 0;
    }
  } else {
    uiGlobalX(offs);
    uiResetStep(50);
    uiGlobalX(offs);
    Label("No devices detected!", 15);
  }
}


void drawTab() {
  if (found) {
    int minX = 9999, minY = 9999, maxX = 0, maxY = 0; /// -----------bad optimisation :((( ---------------
    for (int[] pos : ledPos) {
      minX = min(minX, pos[0]);
      minY = min(minY, pos[1]);
      maxX = max(maxX, pos[0]);
      maxY = max(maxY, pos[1]);
    }

    PGraphics paint;
    paint = createGraphics(width - 2*offs, height - width/6 - 7*_step_y);
    paint.beginDraw();
    paint.background(0);
    paint.stroke(255);
    paint.strokeWeight(2);
    colorMode(HSB);
    if (millis() - drawSendCooldown > 100) {
      drawSendCooldown = millis();
      for (int i = 0; i < ledPos.size(); i++) {
        int x = int(map(ledPos.get(i)[0], minX, maxX, 20, paint.width-20));
        int y = int(map(ledPos.get(i)[1], minY, maxY, 20, paint.height-20));

        if (abs(x-mouseX+offs) < 20 && abs(y-mouseY+offs) < 20) {

          color col = color(hue.value, sat.value, brn.value);

          if (((mousePressed && mouseButton == LEFT) || androidMode) && col != drawColor.get(i)) {
            drawColor.set(i, col);
            sendData(new int[] {5, 1, i/100, i%100, (col >> 16) & 0xFF, (col >> 8) & 0xFF, col & 0xFF});
            //println(i + ": " + (int)hue.value +" "+ (int)sat.value +" "+ (int)brn.value +" "+hex(color(hue.value, sat.value, brn.value)));
            break;
          }

          if (mousePressed && mouseButton == RIGHT && !androidMode && drawColor.get(i) != 0) {
            drawColor.set(i, 0);
            sendData(new int[] {5, 1, i/100, i%100, 0, 0, 0});
             break;
          }
        }
      }
    }
    for (int i = 0; i < ledPos.size(); i++) {
        int x = int(map(ledPos.get(i)[0], minX, maxX, 20, paint.width-20));
        int y = int(map(ledPos.get(i)[1], minY, maxY, 20, paint.height-20));
        paint.fill(drawColor.get(i));
        paint.circle(x, y, 15);
        paint.fill(255);
        paint.text(i+1, x-5, y-10);
    }
    paint.endDraw();
    image(paint, offs, offs);

    stroke(255);
    strokeWeight(2);
    fill(color(hue.value, sat.value, brn.value));
    rect(offs, height - width/6 - 6*_step_y, width - 2*offs, _step_y, 20);
    fill(255);
    text(hex(color(hue.value, sat.value, brn.value)), offs+37, height - width/6 - 4*_step_y - 20);
    colorMode(RGB);

    uiResetStep(height - width/6 - 4*_step_y);
    uiResetX(offs);
    uiGlobalX(offs);
    Label("Hue:", 15);
    Label("Saturation:", 15);
    Label("Brightness:", 15);
    uiResetStep(height - width/6 - 4*_step_y);
    hue.show(0, 255, WW, uiStep(), W);
    sat.show(0, 255, WW, uiStep(), W);
    brn.show(0, 255, WW, uiStep(), W);
    if (Button("Clear")) {
      sendData(new int[] {3, 3});
      for (int i=0; i<drawColor.size(); i++)
        drawColor.set(i, color(0));
    }
    uiResetStep(uiPrevStep());
    if (Button("Apply", 170))
      for (int i = 0; i < ledPos.size(); i++) {
        color col = drawColor.get(i);
        sendData(new int[] {5, 1, i/100, i%100, (col >> 16) & 0xFF, (col >> 8) & 0xFF, col & 0xFF});
        delay(100);
      }
  } else {
    uiGlobalX(offs);
    uiResetStep(50);
    uiGlobalX(offs);
    Label("No devices detected!", 15);
  }
}


void switchCfg() {
  curTab = 0;
  sendData(new int[] {2, 7});
  stopCam();
}
void switchEffects() {
  curTab = 1;
  stopCam();
  int preset = getNewPalette();
  sendData(new int[] {2, 1, 1});
  power.value = true;
  sendData(new int[] {4, 0, preset});
  parseMode = 4;
}
void switchCalib() {
  curTab = 2;
  if (found) startCam();
}
void switchDrw() {
  curTab = 3;
  sendData(new int[] {5, 0});
  stopCam();
  for (int i=0; i<drawColor.size(); i++)
    drawColor.set(i, color(0));
  /*int x = 0;
   int y = 0;
   for (int[] pos: ledPos){
   pos[0] = x;
   pos[1] = y;
   if (++x > 8){
   x = 0;
   y++;
   }
   }*/
}


int getNewPalette(){
  int palette = 0;
  if (current_effs_tab == 0)
    palette = dropPalt1.selected;
  if (current_effs_tab == 1)
    palette = palt1.length + dropPalt2.selected;
  if (current_effs_tab == 2)
    palette = palt1.length + palt2.length + dropPalt3.selected;
  return palette;
}

String getPaletteName(int pal_num){
  String palette = "!" + pal_num + "!";
  if (pal_num < palt1.length)
    palette = palt1[pal_num];
  else if (pal_num - palt1.length < palt2.length)
    palette = palt2[pal_num - palt1.length];
  else if (pal_num - palt1.length - palt2.length < palt3.length)
    palette = palt3[pal_num - palt1.length - palt2.length];
  return palette;
}

String getEffectName(int eff_num){
  String effect = "!" + eff_num + "!";
  if (eff_num < effs.length)
    effect = effs[eff_num];
  return effect;
}
