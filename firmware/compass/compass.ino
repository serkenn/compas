// GPS サテライトコンパス 統合ファームウェア
// ボード: Arduino Nano Every (Arduino megaAVR Boards)
// ライブラリ: TinyGPSPlus (Mikal Hart) — ライブラリマネージャから導入
// 配線: docs/wiring.md 参照
#include <Wire.h>
#include "config.h"
#include "settings.h"
#include "buttons.h"
#include "seg7.h"
#include "gpsnav.h"
#include "mag.h"
#include "heading.h"
#include "rtclock.h"
#include "racetimer.h"
#include "ui.h"

void setup() {
  SettingsStore::load();
  Wire.begin();
  Seg7::begin();
  Buttons::begin();
  Nav::begin();
  Mag::begin(); // 不在でも起動は続行 (GPS のみで動作)
  Clock::begin();
  Ui::begin();
}

void loop() {
  Nav::poll();
  Mag::poll();
  RaceTimer::tick();
  Clock::task();

  BtnEvent e = Buttons::poll();
  if (e != BtnEvent::None) Ui::handle(e);

  uint32_t now = millis();

  static uint32_t lastHeading = 0;
  if (now - lastHeading >= 200) {
    lastHeading = now;
    Heading::update();
    if (Heading::valid()) {
      Seg7::showNumber(Heading::deg());
      Seg7::setBlink(!Heading::usingGps()); // MAG ソース時は周期ブリンク
    } else {
      Seg7::showDashes();
      Seg7::setBlink(false);
    }
  }

  static uint32_t lastRender = 0;
  if (now - lastRender >= 250) {
    lastRender = now;
    Ui::render();
  }
}
