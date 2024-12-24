// Исходник приложения GyverTwink
// Написано на коленке, возможно позже переделаю =(
// v1.0 beta
// v1.1 release
// v1.2 - калибровка больше 255, автоматический масштаб интерфейса, поля ввода подвинул наверх, оптимизация от TheAirBlow 
// v1.3 - опять фиксы масштаба
// v1.6 - починил связь с гирляндой
// v1.7 - порядок в меню, ОПЯТЬ ПОЧИНИЛ СВЯЗЬ
// v2.0 - добавлены эффекты, улучшена калибровка, добавлено функция рисования
// v2.1 - система с выбором эффекта заменена на систему пресетов, в который включен эффект и палитра для него

// ============== ВАЖНО! ===============
// Установить библиотеки из менеджера библиотек:
// (Набросок/Импортировать библиотеку/Добавить библиотеку)
// - Video
// - Ketai

// Установить библиотеки вручную:
// (в documents/processing/libraries)
// - http://ubaa.net/shared/processing/udp/ - download Processing library

// Android/Sketch Permissions установлены
// - CAMERA
// - INTERNET
// - READ_EXTERNAL_STORAGE

/* TODO:
// 1-й записанный светодиод всегда невернный, хотя при чтении из памяти он пропадает
можно добавить эффекты
16.Призмата
19.огонь 2021
21.Тени
30.синусоид (блуждающая точка/прицел)
?70.цветные драже

// lava_colors не работает. возможно ошибки значений настроек (выходят за мин/макс) (но иногда работает, лол)
*/


// ============== НАСТРОЙКИ ===============
// true - Android режим, false - PC режим
private static final boolean androidMode = false;

// для PC режима раскомментируй две строки ниже. Для Android - закомментируй
void openKeyboard() {}
void closeKeyboard() {}

// чтобы сбилдить под Android - нужно установить Android mode
// встроенный билдер собирает под SDK версии 29
// я собирал проект в Android Studio под target 32 версии

// масштаб интерфейса
float androidScale = 2.8;
float pcScale = 1.4;

// ============== LIBRARIES ===============
import processing.video.*;
import hypermedia.net.*;
import ketai.camera.*;
import ketai.net.*;
KetaiCamera Acam;
Capture Wcam;
UDP udp;

// ============== VARIABLES ================
int X = 60;     // размер сетки по X (задан вручную) 1 - 60 - 255
int Y;          // размер сетки по Y (рассчитан автоматически) 1 - 255
int maxX, maxY; // точка максимума в координатах сетки
int size;       // размер "пикселя"
int[][] brMap;  // карты яркости (0,1,2) 
PImage frame, ring; // frame - последний кадр с камеры, ring - кольцо выделения яркой точки
boolean camReady = false;
boolean camStart = false;
String brIP, curIP;
int port = 8888;
boolean searchF, found = false;
byte parseMode = 0;
int actionTmr; // таймер действия
StringList ips = new StringList();

int curPreset = 0;
int curPalette = 0;
int curEffect = 0;

boolean calibF = false; // запущен ли режим калибровки
int calibCount = 0;
int WW, W;
int offs = 30;
String[] file;

int[] lastLedPos = {0,0};
int[] curLedPos = {0,0};
ArrayList<int[]> ledPos = new ArrayList<int[]>();
IntList drawColor = new IntList();
int attempt = 0;
int drawSendCooldown = 0;

// ============== ПРОГРАММА ===============
void settings() {
  if (!androidMode)
    size(600, 1000);
  smooth(8);
}

void setup() {
  androidScale = width/400.0;
  offs = width / 25;
  if (androidMode) W = width/2;
  else W = 300;      
  WW = width-W-offs;

  file = loadStrings("subnet.txt");
  if (file == null) {
    println("Subnet text file is empty");
    file = new String[1];
    file[0] = "255.255.255.0";
    saveStrings("subnet.txt", file);
  }
  subnet.text = file[0];

  if (androidMode) uiSetScale(androidScale);
  else uiSetScale(pcScale);

  udp = new UDP(this);
  udp.listen(true);
  startSearch();
}

void draw() {
  if (searchF) {
    if (millis() - actionTmr > 800) {
      searchF = false;
      if (ips.size() == 0) ips.append("not found");
      else {
        found = true;
        requestCfg();
      }
    }
  } else ui();

  // Калибровка
  if (calibF) {
    if (millis() - actionTmr > 400) { // срабатывает по таймеру действия
      actionTmr = millis();
      if (calibCount == 0) makeMap(0);
      
      if (calibCount > int(leds.text)) {
        calibF = false;
        lastLedPos = curLedPos;
        curLedPos = new int[] {maxX, maxY};
        sendData(new int[] {3, 2, calibCount/100, calibCount%100, curLedPos[0], curLedPos[1]});
        calibCount = 0;
        
        int minX = 9999, minY = 9999, maxX = 0, maxY = 0;
        for (int[] pos : ledPos) {
          minX = min(minX, pos[0]);
          minY = min(minY, pos[1]);
          maxX = max(maxX, pos[0]);
          maxY = max(maxY, pos[1]);
        }
        println("Min: (" + minX + "; " + minY + ") max: (" + maxX + "; " + maxY + ")");
        return;
      }
      
      if (calibCount > 0 && calibCount % 20 == 0) updateBaseFrame(calibCount);
      float dist = sq(curLedPos[0] - lastLedPos[0]) + sq(curLedPos[1] - lastLedPos[1]);
      if (dist < X*X/5 || attempt >= 5 || calibCount == 0){
        attempt = 0;
        lastLedPos = curLedPos;
        if (maxX < 2 && maxY < 2) { maxX = lastLedPos[0] + 1; maxY = lastLedPos[1]; }
        curLedPos = new int[] {maxX, maxY};
        sendData(new int[] {3, 1, calibCount/100, calibCount%100, curLedPos[0], curLedPos[1]});
        ledPos.add(new int[] {curLedPos[0], curLedPos[1]});
        drawColor.append(color(255));
        println("#" + calibCount + ", positionXY: " + curLedPos[0]+ ", " + curLedPos[1]);
        calibCount++;
      }
      else { println("error on #" + calibCount + ", distance: " + dist); attempt++; }
    }
  }
  fill(255);
  text("FPS: " + int(frameRate*100)/100f, 60, 30);
}
