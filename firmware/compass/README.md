# compass — 統合ファームウェア

## ビルド環境

1. Arduino IDE の ボードマネージャで **Arduino megaAVR Boards** を導入し、
   ボードは **Arduino Nano Every** を選択（Registers emulation: None 推奨）
2. ライブラリマネージャで **TinyGPSPlus**（Mikal Hart）を導入
3. `compass.ino` を開いて書き込み

CLI の場合:

```sh
arduino-cli core install arduino:megaavr
arduino-cli lib install TinyGPSPlus
arduino-cli compile --fqbn arduino:megaavr:nona4809 firmware/compass
arduino-cli upload  --fqbn arduino:megaavr:nona4809 -p <port> firmware/compass
```

## モジュール構成

| ファイル | 役割 |
|---|---|
| compass.ino | setup/loop。各モジュールの起動とポーリング |
| config.h | ピンアサイン・しきい値などのハード定数 |
| settings | EEPROM 保存のユーザー設定（ソース・偏差・TZ・磁気オフセット） |
| gpsnav | GT-502 の NMEA 受信（INVEN 反転受け）・1PPS・航程ログ |
| mag | BM1422AGMV 読み出し・ハードアイアン較正 |
| heading | ソース選択（AUTO/GPS/MAG）・円環スムージング・偏差補正 |
| seg7 | 7セグ 3 桁ダイナミック点灯（TCB1 割り込み）・MAG 時ブリンク |
| rtclock | DS1307（UTC 保持）と GPS の時刻同期・TZ 適用 |
| racetimer | レーススタートタイマー（5:00 → 0 → カウントアップ） |
| buttons | 抵抗ラダー 3 ボタンの短押し/長押し/リピート判定 |
| ui | LCD 16×2 の 6 画面 + SETUP |

## 操作

- **MODE 短押し**: NAV → RACE → LOG → CLOCK → POS → STEER → NAV…
- **MODE 長押し**: SETUP に入る / SETUP から保存して抜ける
- **RACE**: A=開始/一時停止、B=分同期（スタート信号に合わせて押す）、B 長押し=リセット
- **LOG**: B 長押し=航程リセット
- **STEER**: A/B=目標針路∓1（押しっぱなしで連続）、A 長押し=現在針路をセット
- **SETUP**: MODE 短押しで項目送り、A/B で値変更。MAG CAL は A で開始→水平で 1〜2 回転→A で確定

## 実機合わせが必要な箇所

- `config.h` のボタンしきい値（t08 の実測値）
- `mag.cpp` の軸の向き（`atan2(-y, x)`）と `config.h` の取付オフセット
- BM1422 のレジスタ初期化値はデータシートと突き合わせて要検証（t05 で確認）
- 7セグが暗い/明るい場合は 330Ω を変更（docs/wiring.md §5 の計算参照）
