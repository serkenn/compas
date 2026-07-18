#pragma once
#include "buttons.h"

// LCD 16x2 + ボタンのモード UI
namespace Ui {
void begin();
void handle(BtnEvent e);
void render(); // 250ms 間隔で呼ぶ
}
