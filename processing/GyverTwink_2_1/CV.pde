// найти разницу
void findMax() {
  int[] maxes = getMax();
  maxX = maxes[0]; maxY = maxes[1];
  PGraphics buf = createGraphics(frame.width, frame.height);
  buf.beginDraw();
  //buf.text(str(maxes[2]) + " " + maxX + " " + maxY, 10, 10);
  //buf.text(lastLedPos[0] + " " + lastLedPos[1], 10, 20);
  buf.stroke(0xFFFF0000);
  buf.strokeWeight(2);
  buf.noFill();
  buf.circle(maxX*size+size/2, maxY*size+size/2, 30);
  
  buf.stroke(0xFF00C000);
  buf.strokeWeight(2);
  int i = 1;
  for (int[] pos: ledPos){
    buf.text(i, pos[0]*size+size/2-7, pos[1]*size+size/2-5);
    buf.circle(pos[0]*size+size/2, pos[1]*size+size/2, 7);
    i++;
  }
  buf.endDraw();
  ring = buf.get();
}

// создать карту яркости (0 базовая, 1 текущая, 2 разностная) из frame
void makeMap(int i) {
  for (int y = 0; y < Y; y++) {
    for (int x = 0; x < X; x++) {
      int sum = 0;
      for (int yy = 0; yy < size; yy++) {
        for (int xx = 0; xx < size; xx++) {
          int pos = (y*size + yy) * frame.width + (x*size + xx);
          int col = frame.pixels[pos];
          //sum += col >> 16 & 0xFF;
          sum += (col & 0xFF) / 3 + (col >> 8 & 0xFF) / 3 + (col >> 16 & 0xFF) / 3; //разделение на байты по цветам
        }
      }
      sum /= size * size;
      brMap[y*X + x][i] = sum;
    }
  }
}

// вывести карту яркости (0 базовая, 1 текущая, 2 разностная) из brMap
PGraphics drawMap(int m) {
  PGraphics buf = createGraphics(frame.width, frame.height);
  buf.beginDraw();
  buf.noStroke();
  for (int y = 0; y < Y; y++) {
    for (int x = 0; x < X; x++) {
      int col = brMap[y*X + x][m];
      buf.fill(col);
      buf.rect(x*size, y*size, size, size);
      
      //buf.fill(color(255,255,255));
      //buf.text(str(brMap[y*X + x][m]), x*size+size/2, y*size+size/2);
    }
  }
  buf.endDraw();
  return buf;
}

int[] getMax() {
  int maxBrightness = 0;
  int[] brightestCoords = new int[3];

  for (int x = 0; x < X; x++) {
    for (int y = 0; y < Y; y++) {
      boolean[][] visited = new boolean[X][Y];
      int brightness = dfs(visited, x, y, 1);
      brMap[y * X + x][3] = brightness;
      
      if (brightness > maxBrightness) {
        maxBrightness = brightness;
        brightestCoords = new int[]{x, y, maxBrightness}; // Сохраняем координаты первой найденной точки группы
      }
    }
  }
  return brightestCoords;
}


private final int[][] DIRECTIONS = {
  {-1, 0}, {1, 0}, {0, -1}, {0, 1} // Up, Down, Left, Right
};

private int dfs(boolean[][] visited, int x, int y, float factor) {
  if (x < 0 || x >= X || y < 0 || y >= Y || visited[x][y] || factor < 0.05) {
    return 0;
  }

  visited[x][y] = true;
  int i = y * X + x;
  brMap[i][2] = (brMap[i][1] - brMap[i][0]);
  brMap[i][2] = max(brMap[i][2], 0);
  int brightness = (int)(brMap[i][2]*factor);
  for (int[] direction : DIRECTIONS) {
    brightness += dfs(visited, x + direction[0], y + direction[1], factor*0.3);
  }
  
  brightness = min(brightness / 2, 255);
  return brightness;
}

void updateBaseFrame(int led_id){
  sendData(new int[] {3, 3});
  delay(50);
  makeMap(0);
  sendData(new int[] {3, 4, led_id/100, led_id%100});
  delay(50);
  actionTmr += 100;
}
