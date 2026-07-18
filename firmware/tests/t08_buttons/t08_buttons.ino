// t08: ボタンラダー (A7) 確認
// 生の ADC 値と判定結果を表示する。実測値を docs/wiring.md §6 のしきい値に反映すること。

void setup() {
  Serial.begin(115200);
}

const char *judge(int v) {
  if (v > 800) return "MODE";
  if (v > 430 && v <= 650) return "A";
  if (v > 250 && v <= 430) return "B";
  if (v < 150) return "-";
  return "?"; // しきい値の隙間 → 要調整
}

void loop() {
  int v = analogRead(A7);
  Serial.print(F("ADC="));
  Serial.print(v);
  Serial.print(F("  -> "));
  Serial.println(judge(v));
  delay(200);
}
