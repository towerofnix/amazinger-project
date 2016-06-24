#include <nds.h>
#include <stdlib.h>
#include <stdio.h>
#include <gl2d.h>

#include "chip8.hpp"

bool keyboardShown = true;
int keyboardToggled = false;
bool input[0xF];
touchPosition touch;

void waitFrames(unsigned int frames) {
  for (unsigned int i = 0; i < frames; i++) {
    swiWaitForVBlank();
  }
}

void doKey(unsigned int n, unsigned int x, unsigned int y) {
  if (touch.px > x && touch.px < x + 20 &&
      touch.py > y && touch.py < y + 20) {
    iprintf("Click! %d\n", n);
    input[n] = true;
  } else {
    input[n] = false;
  }

  int color = RGB15(31, 31, 31);
  if (input[n]) {
    color = RGB15(27, 27, 27);
  }

  glBoxFilled(x, y, x + 20, y + 20, color);
}

int main(void) {
  videoSetMode(MODE_5_3D);
  vramSetBankA(VRAM_A_MAIN_BG);
  glScreen2D();
  consoleDemoInit();

  // int bg = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);
  // u16* backBuffer = (u16*)bgGetGfxPtr(bg) + 256*256;
  bool gfxBuf[64 * 32];
  memset(gfxBuf, false, 64 * 32);
  chip8::gfx_buffer(gfxBuf);

  chip8::initialize();

  unsigned int size = (13 + 1) * 2;
  unsigned short buffer[size] = {
    0x60, 0x00,
    0x00, 0xE0,
    0x61, 0x0F,  // wrap 10 to 0
    0x82, 0x00,  // ^
    0x80, 0x17,  // ^
    0x3F, 0x00,  // ^ carried, V0 = 0
     0x60, 0x00, // ^
    0x4F, 0x00,  // ^ didn't carry, restore value
     0x80, 0x20, // ^
    0xF0, 0x29,  // draw character V0
    0xDE, 0xE5,  // ^
    0x70, 0x01,
    0x12, 0x02
  };
  chip8::loadProgram(buffer, size);
  int status = 0;

  while (1) {
    touchRead(&touch);
    scanKeys();

    if (status == 0) {
      status = chip8::emulateCycle();
    } else if (status == 1) {
      iprintf("Status = 1\n");
      status = -1; // don't show message again
    }

    glBegin2D();
    // if (keyboardShown) {
    //   glBoxFilled(72, 47, 173, 158, RGB15(17, 17, 17));
    //   glBoxFilled(72, 149, 173, 158, RGB15(15, 15, 15));
    //   doKey(0x1, 75, 50);
    //   doKey(0x2, 100, 50);
    //   doKey(0x3, 125, 50);
    //   doKey(0xC, 150, 50);
    //   doKey(0x4, 75, 75);
    //   doKey(0x5, 100, 75);
    //   doKey(0x6, 125, 75);
    //   doKey(0xD, 150, 75);
    //   doKey(0x7, 75, 100);
    //   doKey(0x8, 100, 100);
    //   doKey(0x9, 125, 100);
    //   doKey(0xE, 150, 100);
    //   doKey(0xA, 75, 125);
    //   doKey(0x0, 100, 125);
    //   doKey(0xB, 125, 125);
    //   doKey(0xF, 150, 125);
    // }
    // if (keysHeld() & KEY_R) {
    //   if (keyboardToggled == 0) {
    //     keyboardShown = !keyboardShown;
    //     keyboardToggled = 10;
    //   }
    // } else {
    //   keyboardToggled = 0;
    // }
    // if (keyboardToggled) keyboardToggled--;
    glBoxFilled(0, 0, 256, 192, RGB15(7, 7, 7));
    for (int y = 0; y < 32; y++) {
      for (int x = 0; x < 64; x++) {
        bool on = gfxBuf[y * 64 + x];
        int x1 = (256 / 2 - 64 * 3 / 2) + x * 3;
        int x2 = x1 + 3;
        int y1 = (192 / 2 - 32 * 3 / 2) + y * 3;
        int y2 = y1 + 3;
        if (on) {
          glBoxFilled(x1, y1, x2, y2, RGB15(31, 31, 31));
        } else {
          glBoxFilled(x1, y1, x2, y2, RGB15(0, 0, 0));
        }
      }
    }
    glEnd2D();
    glFlush(0);

    waitFrames(1);
  }

  return 0;
}
