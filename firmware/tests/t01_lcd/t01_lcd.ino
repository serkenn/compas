// t01: LCD 16x2 単体確認 (4bit モード)
// 配線: docs/wiring.md §4。コントラストは半固定 10kΩ で調整。
#include <LiquidCrystal.h>

LiquidCrystal lcd(3, 4, 5, 6, 7, 8); // RS, E, DB4-DB7

void setup() {
  lcd.begin(16, 2);
  lcd.print("GPS COMPASS t01");
  lcd.setCursor(0, 1);
  lcd.print("LCD OK \xDF"); // \xDF = ° (HD44780 A00 ROM)
}

void loop() {
  lcd.setCursor(9, 1);
  lcd.print(millis() / 1000);
  delay(200);
}
