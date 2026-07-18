#include "gpsnav.h"
#include "config.h"
#include <TinyGPSPlus.h>

namespace Nav {

static TinyGPSPlus gps;

static volatile uint32_t ppsMillis = 0;
static void ppsIsr() { ppsMillis = millis(); }

// 航程ログ
static float trip_m = 0;
static float maxKt_ = 0;
static uint32_t movingSecs = 0;
static double lastLat = 0, lastLng = 0;
static bool lastPosSet = false;
static uint32_t lastLogMs = 0;

void begin() {
  Serial1.begin(GPS_BAUD);
  // GPS TX は 2SC1815 反転バッファ経由 → RX1(D1=PC5) の論理を INVEN で反転して受ける。
  // レジスタ直書きだと ATMEGA328 レジスタエミュレーション有効時に衝突するため、コアのマクロ経由で触る。
  PORT_t *rxPort = digitalPinToPortStruct(PIN_GPS_RX);
  volatile uint8_t *rxCtrl = getPINnCTRLregister(rxPort, digitalPinToBitPosition(PIN_GPS_RX));
  if (rxCtrl) *rxCtrl |= PORT_INVEN_bm;
  pinMode(PIN_PPS, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PPS), ppsIsr, FALLING);
}

static void updateLog() {
  uint32_t now = millis();
  if (now - lastLogMs < 3000) return;
  uint32_t dt = (now - lastLogMs) / 1000;
  lastLogMs = now;

  if (!posValid()) { lastPosSet = false; return; }
  if (sogValid() && sogKt() > maxKt_) maxKt_ = sogKt();

  double la = lat(), ln = lng();
  if (lastPosSet && sogValid() && sogKt() >= LOG_MIN_SOG_KT) {
    trip_m += TinyGPSPlus::distanceBetween(lastLat, lastLng, la, ln);
    movingSecs += dt;
  }
  lastLat = la; lastLng = ln; lastPosSet = true;
}

void poll() {
  while (Serial1.available()) gps.encode(Serial1.read());
  updateLog();
}

bool hasFix() { return gps.location.isValid() && gps.location.age() < GPS_STALE_MS; }
uint8_t sats() { return gps.satellites.isValid() ? gps.satellites.value() : 0; }
bool sogValid() { return gps.speed.isValid() && gps.speed.age() < GPS_STALE_MS; }
float sogKt() { return gps.speed.knots(); }
bool cogValid() { return gps.course.isValid() && gps.course.age() < GPS_STALE_MS; }
float cogDeg() { return gps.course.deg(); }
bool posValid() { return hasFix(); }
double lat() { return gps.location.lat(); }
double lng() { return gps.location.lng(); }

bool timeValid() {
  return gps.date.isValid() && gps.time.isValid() &&
         gps.time.age() < GPS_STALE_MS && gps.date.year() >= 2026;
}

void getUtc(uint16_t &y, uint8_t &mo, uint8_t &d, uint8_t &h, uint8_t &mi, uint8_t &s) {
  y = gps.date.year(); mo = gps.date.month(); d = gps.date.day();
  h = gps.time.hour(); mi = gps.time.minute(); s = gps.time.second();
}

uint32_t ppsAgeMs() {
  noInterrupts();
  uint32_t t = ppsMillis;
  interrupts();
  if (t == 0) return UINT32_MAX;
  return millis() - t;
}

float tripNm() { return trip_m / 1852.0f; }
float maxKt() { return maxKt_; }
float avgKt() { return movingSecs ? tripNm() / (movingSecs / 3600.0f) : 0; }
void resetLog() { trip_m = 0; maxKt_ = 0; movingSecs = 0; lastPosSet = false; }

} // namespace Nav
