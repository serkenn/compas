#include "racetimer.h"

namespace RaceTimer {

static const int32_t PRESET_S = 5 * 60;

enum class St : uint8_t { Idle, Count, CountPause, Up, UpPause };
static St st = St::Idle;
static int32_t value = PRESET_S; // Count系: 残り秒 / Up系: 経過秒
static uint32_t lastMs = 0;
static uint32_t accMs = 0;

void tick() {
  uint32_t now = millis();
  if (st != St::Count && st != St::Up) { lastMs = now; return; }
  accMs += now - lastMs;
  lastMs = now;
  while (accMs >= 1000) {
    accMs -= 1000;
    if (st == St::Count) {
      if (--value <= 0) { st = St::Up; value = 0; } // スタート! → カウントアップへ
    } else {
      value++;
    }
  }
}

void toggle() {
  switch (st) {
    case St::Idle: value = PRESET_S; accMs = 0; lastMs = millis(); st = St::Count; break;
    case St::Count: st = St::CountPause; break;
    case St::CountPause: lastMs = millis(); st = St::Count; break;
    case St::Up: st = St::UpPause; break;
    case St::UpPause: lastMs = millis(); st = St::Up; break;
  }
}

void sync() {
  if (st != St::Count && st != St::CountPause) return;
  int32_t rounded = ((value + 30) / 60) * 60;
  accMs = 0;
  if (rounded <= 0) { st = St::Up; value = 0; }
  else value = rounded;
}

void reset() {
  st = St::Idle;
  value = PRESET_S;
  accMs = 0;
}

bool idle() { return st == St::Idle; }
bool inCountdown() { return st == St::Count || st == St::CountPause; }
bool running() { return st == St::Count || st == St::Up; }
int32_t seconds() { return value; }

} // namespace RaceTimer
