#include "settings.h"
#include <EEPROM.h>

Settings settings;

static const uint8_t MAGIC = 0x5A;
static const uint8_t VERSION = 1;

namespace SettingsStore {

static void defaults() {
  settings.magic = MAGIC;
  settings.version = VERSION;
  settings.source = SRC_AUTO;
  settings.sogThTenths = 10;      // 1.0kt
  settings.variationTenths = -75; // 西偏 7.5° (関東近海の目安)
  settings.dispMagnetic = 0;
  settings.tzMinutes = 9 * 60;    // JST
  settings.magOffX = 0;
  settings.magOffY = 0;
}

void load() {
  EEPROM.get(0, settings);
  if (settings.magic != MAGIC || settings.version != VERSION) {
    defaults();
    save();
  }
}

void save() {
  EEPROM.put(0, settings);
}

} // namespace SettingsStore
