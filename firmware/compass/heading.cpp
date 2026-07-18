#include "heading.h"
#include "gpsnav.h"
#include "mag.h"
#include "settings.h"
#include <math.h>

namespace Heading {

static bool valid_ = false;
static bool usingGps_ = false;
static float fx = 0, fy = 0; // 平滑化した単位ベクトル

static float wrap360(float d) {
  while (d < 0) d += 360.0f;
  while (d >= 360.0f) d -= 360.0f;
  return d;
}

void update() {
  bool gpsOk = Nav::hasFix() && Nav::cogValid() && Nav::sogValid() &&
               Nav::sogKt() >= settings.sogThTenths / 10.0f;
  bool magOk = Mag::healthy();

  bool useGps;
  switch (settings.source) {
    case SRC_GPS: useGps = true; magOk = false; break;
    case SRC_MAG: useGps = false; gpsOk = false; break;
    default:      useGps = gpsOk; break; // AUTO: GPS 優先
  }

  float raw;
  float variation = settings.variationTenths / 10.0f;
  if (useGps && gpsOk) {
    // COG は真方位。磁方位表示なら偏差を引く
    raw = Nav::cogDeg() - (settings.dispMagnetic ? variation : 0);
    usingGps_ = true;
  } else if (magOk) {
    // 地磁気は磁方位。真方位表示なら偏差を足す
    raw = Mag::magneticHeading() + (settings.dispMagnetic ? 0 : variation);
    usingGps_ = false;
  } else {
    valid_ = false;
    fx = fy = 0;
    return;
  }
  raw = wrap360(raw);

  float rad = raw * PI / 180.0f;
  float cx = cos(rad), cy = sin(rad);
  float mag2 = fx * fx + fy * fy;
  float dot = fx * cx + fy * cy; // |f|*cos(角度差)
  if (!valid_ || mag2 < 0.04f || dot < sqrt(mag2) * 0.5f) {
    // 初回・ソース切替直後・60° 超の急変はスナップ
    fx = cx; fy = cy;
  } else {
    const float a = 0.3f;
    fx += a * (cx - fx);
    fy += a * (cy - fy);
  }
  valid_ = true;
}

bool valid() { return valid_; }
bool usingGps() { return usingGps_; }

uint16_t deg() {
  float d = atan2(fy, fx) * 180.0f / PI;
  uint16_t v = (uint16_t)lround(wrap360(d));
  return v % 360;
}

} // namespace Heading
