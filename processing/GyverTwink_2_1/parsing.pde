static private final boolean debug = true;
void receive(byte[] ubuf) {
  if (ubuf[0] != 'G' || ubuf[1] != 'T') return;
  int[] data = new int[1000];
  if(debug) print("recived: G T ");
  for (int i = 0; i < ubuf.length - 2; i++) {
    data[i] = int(ubuf[i+2]);
    if(debug) print(data[i] + " ");
  }
  if(debug) println();
  if (parseMode != data[0]) return;
  
  switch (data[0]) {
  case 0: // Поиск
    String ip = brIP.substring(0, brIP.lastIndexOf('.')+1) + str(data[1]);
    if (!ips.hasValue(ip)) ips.append(ip);
    break;

  case 1: // Настройки
    if (data[1] == 0){
      searchF = false;
      leds.text = str(data[2] * 100 + data[3]);
      power.value = boolean(data[4]);
      bri.value = data[5];
      auto.value = boolean(data[6]);
      rnd.value = boolean(data[7]);
      prd.value = data[8];
      offT.value = boolean(data[9]);
      offS.value = data[10];
    }
    if (data[1] >= 1){
      if (data[1] == 1){
        ledPos = new ArrayList<int[]>();
        drawColor = new IntList();
      }
      int dataChunk = data[1] - 1;
      print("dataChunk #" + dataChunk + " (" + (2 + min(dataChunk * 120 + 120, int(leds.text)*2) - dataChunk * 120) +")" + ": ");
      for (int i = 2; i < (2 + min(dataChunk * 120 + 120, int(leds.text)*2) - dataChunk * 120); i+=2) {
        ledPos.add(new int[] {data[i], data[i+1]});
        drawColor.append(color(255));
        print("("+data[i] + ", " + data[i+1]+"); "); //<>//
      }
      println();
    }
  break;

  case 4: // Эффект
    fav.value = boolean(data[1]);
    scl.value = data[2];
    spd.value = data[3];    
    curEffect = data[4];
    curPalette = data[5];
    break;
  }
}
