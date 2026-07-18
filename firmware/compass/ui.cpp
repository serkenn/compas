#include "ui.h"
#include "config.h"
#include "settings.h"
#include "gpsnav.h"
#include "heading.h"
#include "mag.h"
#include "rtclock.h"
#include "racetimer.h"
#include <LiquidCrystal.h>

namespace Ui {

static LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// MODE 短押しで巡回する画面
enum class Mode : uint8_t { Nav, Race, Log, Clk, Pos, Steer, COUNT };
static Mode mode = Mode::Nav;

static bool inSetup = false;
enum : uint8_t { SET_SRC, SET_SOGTH, SET_VAR, SET_DISP, SET_TZ, SET_CAL, SET_COUNT };
static uint8_t setupItem = SET_SRC;

static uint16_t steerTarget = 0;

// 16 桁に空白パディングして 1 行描画
static void line(uint8_t row, const char *s) {
  char buf[17];
  uint8_t i = 0;
  for (; i < 16 && s[i]; i++) buf[i] = s[i];
  for (; i < 16; i++) buf[i] = ' ';
  buf[16] = '\0';
  lcd.setCursor(0, row);
  lcd.print(buf);
}

void begin() {
  lcd.begin(16, 2);
  line(0, "GPS COMPASS");
  line(1, "booting...");
}

// ---------- ボタン処理 ----------

static void adjustSetup(int8_t dir) {
  switch (setupItem) {
    case SET_SRC:
      settings.source = (settings.source + 3 + dir) % 3;
      break;
    case SET_SOGTH: {
      int v = settings.sogThTenths + dir;
      settings.sogThTenths = constrain(v, 0, 50);
      break;
    }
    case SET_VAR: {
      int v = settings.variationTenths + dir * 5; // 0.5° 刻み
      settings.variationTenths = constrain(v, -300, 300);
      break;
    }
    case SET_DISP:
      settings.dispMagnetic = !settings.dispMagnetic;
      break;
    case SET_TZ: {
      int v = settings.tzMinutes + dir * 30;
      settings.tzMinutes = constrain(v, -12 * 60, 14 * 60);
      break;
    }
    default:
      break;
  }
}

static void handleSetup(BtnEvent e) {
  switch (e) {
    case BtnEvent::ModeShort:
      setupItem = (setupItem + 1) % SET_COUNT;
      break;
    case BtnEvent::ModeLong: // 保存して終了
      if (Mag::calActive()) Mag::calStop(false);
      SettingsStore::save();
      inSetup = false;
      break;
    case BtnEvent::AShort: case BtnEvent::ARepeat:
      if (setupItem == SET_CAL) {
        if (e == BtnEvent::AShort) Mag::calActive() ? Mag::calStop(true) : Mag::calStart();
      } else adjustSetup(-1);
      break;
    case BtnEvent::BShort: case BtnEvent::BRepeat:
      if (setupItem == SET_CAL) {
        if (e == BtnEvent::BShort) Mag::calStop(false); // 破棄
      } else adjustSetup(+1);
      break;
    default:
      break;
  }
}

void handle(BtnEvent e) {
  if (inSetup) { handleSetup(e); return; }

  if (e == BtnEvent::ModeShort) {
    mode = (Mode)(((uint8_t)mode + 1) % (uint8_t)Mode::COUNT);
    return;
  }
  if (e == BtnEvent::ModeLong) {
    inSetup = true;
    setupItem = SET_SRC;
    return;
  }

  switch (mode) {
    case Mode::Race:
      if (e == BtnEvent::AShort) RaceTimer::toggle();
      else if (e == BtnEvent::BShort) RaceTimer::sync();
      else if (e == BtnEvent::BLong) RaceTimer::reset();
      break;
    case Mode::Log:
      if (e == BtnEvent::BLong) Nav::resetLog();
      break;
    case Mode::Steer:
      if (e == BtnEvent::AShort || e == BtnEvent::ARepeat)
        steerTarget = (steerTarget + 359) % 360;
      else if (e == BtnEvent::BShort || e == BtnEvent::BRepeat)
        steerTarget = (steerTarget + 1) % 360;
      else if (e == BtnEvent::ALong && Heading::valid())
        steerTarget = Heading::deg(); // 現在針路をターゲットに
      break;
    default:
      break;
  }
}

// ---------- 描画 ----------

static void renderNav() {
  char b[24];
  if (Heading::valid())
    snprintf(b, sizeof(b), "HDG %03u\xDF SRC %s", Heading::deg(),
             Heading::usingGps() ? "GPS" : "MAG");
  else
    snprintf(b, sizeof(b), "HDG ---  SRC ---");
  line(0, b);

  if (Nav::sogValid()) {
    uint16_t st = (uint16_t)(Nav::sogKt() * 10 + 0.5f);
    snprintf(b, sizeof(b), "SOG %2u.%ukt SAT%2u", st / 10, st % 10, Nav::sats());
  } else {
    snprintf(b, sizeof(b), "SOG --.-kt SAT%2u", Nav::sats());
  }
  line(1, b);
}

static void renderRace() {
  char b[24];
  int32_t s = RaceTimer::seconds();
  if (RaceTimer::idle()) {
    snprintf(b, sizeof(b), "RACE READY %02ld:%02ld", (long)(s / 60), (long)(s % 60));
    line(0, b);
    line(1, "A:START");
  } else if (RaceTimer::inCountdown()) {
    snprintf(b, sizeof(b), "RACE %c -%02ld:%02ld", RaceTimer::running() ? ' ' : 'P',
             (long)(s / 60), (long)(s % 60));
    line(0, b);
    line(1, "A:S/S B:SYNC");
  } else {
    snprintf(b, sizeof(b), "RACE %c+%01ld:%02ld:%02ld", RaceTimer::running() ? ' ' : 'P',
             (long)(s / 3600), (long)((s / 60) % 60), (long)(s % 60));
    line(0, b);
    line(1, "A:S/S LongB:RST");
  }
}

static void renderLog() {
  char b[24];
  uint32_t t100 = (uint32_t)(Nav::tripNm() * 100 + 0.5f);
  snprintf(b, sizeof(b), "TRIP %4lu.%02lunm", (unsigned long)(t100 / 100),
           (unsigned long)(t100 % 100));
  line(0, b);
  uint16_t mx = (uint16_t)(Nav::maxKt() * 10 + 0.5f);
  uint16_t av = (uint16_t)(Nav::avgKt() * 10 + 0.5f);
  snprintf(b, sizeof(b), "MAX%3u.%u AVG%3u.%u", mx / 10, mx % 10, av / 10, av % 10);
  line(1, b);
}

static void renderClock() {
  char b[24];
  ClockTime t;
  if (Clock::getLocal(t)) {
    snprintf(b, sizeof(b), "%02u:%02u:%02u     %s", t.hour, t.minute, t.second,
             Clock::sourceIsGps() ? "GPS" : "RTC");
    line(0, b);
    int8_t tzh = settings.tzMinutes / 60;
    uint8_t tzm = abs(settings.tzMinutes) % 60;
    snprintf(b, sizeof(b), "%04u-%02u-%02u %+d:%02u", t.year, t.month, t.day, tzh, tzm);
    line(1, b);
  } else {
    line(0, "--:--:--");
    line(1, "NO TIME SOURCE");
  }
}

static void renderPos() {
  char b[24];
  if (Nav::posValid()) {
    double la = Nav::lat(), ln = Nav::lng();
    int lad = (int)fabs(la);
    uint32_t lam = (uint32_t)((fabs(la) - lad) * 60000 + 0.5); // 分×1000
    snprintf(b, sizeof(b), "%c %2d\xDF%02lu.%03lu'", la >= 0 ? 'N' : 'S', lad,
             (unsigned long)(lam / 1000), (unsigned long)(lam % 1000));
    line(0, b);
    int lnd = (int)fabs(ln);
    uint32_t lnm = (uint32_t)((fabs(ln) - lnd) * 60000 + 0.5);
    snprintf(b, sizeof(b), "%c%3d\xDF%02lu.%03lu'", ln >= 0 ? 'E' : 'W', lnd,
             (unsigned long)(lnm / 1000), (unsigned long)(lnm % 1000));
    line(1, b);
  } else {
    line(0, "POS: NO FIX");
    char s[17];
    snprintf(s, sizeof(s), "SAT%2u", Nav::sats());
    line(1, s);
  }
}

static void renderSteer() {
  char b[24];
  if (Heading::valid())
    snprintf(b, sizeof(b), "TGT %03u\xDF HDG %03u", steerTarget, Heading::deg());
  else
    snprintf(b, sizeof(b), "TGT %03u\xDF HDG ---", steerTarget);
  line(0, b);

  if (!Heading::valid()) { line(1, "A:-1 B:+1 LA:SET"); return; }
  int turn = ((int)steerTarget - (int)Heading::deg() + 540) % 360 - 180;
  if (abs(turn) <= 2) {
    line(1, "   ON COURSE");
  } else if (turn > 0) { // 面舵 (右)
    int n = min(6, turn / 10 + 1);
    snprintf(b, sizeof(b), "R%4d\xDF %.*s", turn, n, ">>>>>>");
    line(1, b);
  } else { // 取舵 (左)
    int n = min(6, -turn / 10 + 1);
    snprintf(b, sizeof(b), "%.*s L%4d\xDF", n, "<<<<<<", -turn);
    line(1, b);
  }
}

static void renderSetup() {
  char b[24];
  static const char *names[SET_COUNT] = {"SOURCE", "SOG TH", "VARIATN", "DISP", "TZ", "MAG CAL"};
  snprintf(b, sizeof(b), "SETUP %u/%u %s", setupItem + 1, SET_COUNT, names[setupItem]);
  line(0, b);
  switch (setupItem) {
    case SET_SRC: {
      static const char *src[3] = {"AUTO", "GPS", "MAG"};
      snprintf(b, sizeof(b), "> %s", src[settings.source % 3]);
      break;
    }
    case SET_SOGTH:
      snprintf(b, sizeof(b), "> %u.%u kt", settings.sogThTenths / 10, settings.sogThTenths % 10);
      break;
    case SET_VAR: {
      int v = settings.variationTenths;
      snprintf(b, sizeof(b), "> %c %d.%u\xDF", v >= 0 ? 'E' : 'W', abs(v) / 10, abs(v) % 10);
      break;
    }
    case SET_DISP:
      snprintf(b, sizeof(b), "> %s", settings.dispMagnetic ? "MAGNETIC" : "TRUE");
      break;
    case SET_TZ:
      snprintf(b, sizeof(b), "> %+d:%02u", settings.tzMinutes / 60,
               (unsigned)(abs(settings.tzMinutes) % 60));
      break;
    case SET_CAL:
      if (Mag::calActive())
        snprintf(b, sizeof(b), "TURN! n=%u A:OK", Mag::calCount());
      else
        snprintf(b, sizeof(b), "> A:START");
      break;
  }
  line(1, b);
}

void render() {
  if (inSetup) { renderSetup(); return; }
  switch (mode) {
    case Mode::Nav: renderNav(); break;
    case Mode::Race: renderRace(); break;
    case Mode::Log: renderLog(); break;
    case Mode::Clk: renderClock(); break;
    case Mode::Pos: renderPos(); break;
    case Mode::Steer: renderSteer(); break;
    default: break;
  }
}

} // namespace Ui
