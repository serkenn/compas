// t02: GPS GT-502MGG-N 受信確認
// GPS TX は 2SC1815 反転バッファ経由で D1(RX1=PC5) へ (docs/wiring.md §2)。
// 反転して届くので RX1 の INVEN で論理を戻す。
// 1PPS も反転バッファ経由で D2 へ (FALLING が秒頭)。
// USB シリアルモニタ (115200bps) に NMEA をそのまま流し、1 秒ごとに PPS 回数を表示。

volatile uint16_t ppsCount = 0;

void ppsIsr() { ppsCount++; }

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  // D1=PC5(RX1): 反転バッファ分を INVEN で論理反転 (エミュレーション設定でも通るようマクロ経由)
  PORT_t *rxPort = digitalPinToPortStruct(1);
  volatile uint8_t *rxCtrl = getPINnCTRLregister(rxPort, digitalPinToBitPosition(1));
  if (rxCtrl) *rxCtrl |= PORT_INVEN_bm;
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), ppsIsr, FALLING);
  Serial.println(F("--- t02 GPS echo ---"));
}

void loop() {
  while (Serial1.available()) Serial.write(Serial1.read());
  static uint32_t last = 0;
  if (millis() - last >= 5000) {
    last = millis();
    Serial.print(F("### PPS count: "));
    Serial.println(ppsCount);
  }
}
