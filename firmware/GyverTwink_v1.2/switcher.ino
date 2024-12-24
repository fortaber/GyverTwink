void switchEff() {
  // поиск favorite эффектов
  while (true) {
    if (cfg.rndCh) curPreset = random(0, PRESETS_SIZE);
    else {
      if (++curPreset >= PRESETS_SIZE) curPreset = 0;
    }
    if (presets[curPreset].fav) break;
  }
  DEBUG("switch to: ");
  DEBUGLN(curPreset);
}
