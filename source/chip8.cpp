#include <nds.h>
#include <stdlib.h>
#include <stdio.h>
#include <bitset>

#include "chip8.h"

/* System memory map:
 *
 *   0x000-0x1FF - Chip 8 Interpreter (contains font in emu)
 *   0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
 *   0x200-0xFFF - Program ROM and work RAM
 */

unsigned short opcode;

unsigned char memory[4096];

unsigned char V[16];

unsigned short I;
unsigned short pc;

unsigned char gfx[64 * 32];

unsigned char delay_timer;
unsigned char sound_timer;

unsigned short stack[16];
unsigned short sp;

unsigned char key[16];

bool* gfxBuf;

void initialize_font() {
  unsigned int size = 0x50;
  unsigned char font_data[size] = {
    /* 0 */ 0xF0, 0x90, 0x90, 0x90, 0xF0,
    /* 1 */ 0x20, 0x60, 0x20, 0x20, 0x70,
    /* 2 */ 0xF0, 0x10, 0xF0, 0x80, 0xF0,
    /* 3 */ 0xF0, 0x10, 0xF0, 0x10, 0xF0,
    /* 4 */ 0x90, 0x90, 0xF0, 0x10, 0x10,
    /* 5 */ 0xF0, 0x80, 0xF0, 0x10, 0xF0,
    /* 6 */ 0xF0, 0x80, 0xF0, 0x90, 0xF0,
    /* 7 */ 0xF0, 0x10, 0x20, 0x40, 0x40,
    /* 8 */ 0xF0, 0x90, 0xF0, 0x90, 0xF0,
    /* 9 */ 0xF0, 0x90, 0xF0, 0x10, 0xF0,
    /* A */ 0xF0, 0x90, 0xF0, 0x90, 0x90,
    /* B */ 0xE0, 0x90, 0xE0, 0x90, 0xE0,
    /* C */ 0xF0, 0x80, 0x80, 0x80, 0xF0,
    /* D */ 0xE0, 0x90, 0x90, 0x90, 0xE0,
    /* E */ 0xF0, 0x80, 0xF0, 0x80, 0xF0,
    /* F */ 0xF0, 0x80, 0xF0, 0x80, 0x80,
  };
  for (unsigned int i = 0; i < size; i++) {
    iprintf("Font load: 0x%X %X 0x%X\n", 0x50 + i, i, font_data[i]);
    memory[0x50 + i] = font_data[i];
  }
}

void chip8::gfx_buffer(bool* gfxBuffer) {
  gfxBuf = gfxBuffer;
}

void chip8::initialize() {
  pc = 0x200;
  opcode = 0;
  I = 0;
  sp = 0;
  initialize_font();
}

void chip8::loadProgram(unsigned short* buffer, int buffer_size) {
  for (int i = 0; i < buffer_size; i++) {
    iprintf("Prog load %X = %02X %s\n", i + 512, buffer[i],
            std::bitset<8>(buffer[i]).to_string('0', '1').c_str());
    memory[i + 512] = buffer[i];
  }
}

int chip8::emulateCycle() {
  // Fetch opcode
  opcode = memory[pc] << 8 | memory[pc + 1];

  // iprintf("pc: 0x%X\n", pc);
  // iprintf("opcode: 0x%X\n", opcode);

  // Execute opcode
  switch(opcode & 0xF000) {
    case 0x0000:
      switch(opcode) {
        case 0x00E0: {
          pc += 2;
          iprintf("Clear screen\n");
          for (int i = 0; i < 32 * 64; i++) {
            gfxBuf[i] = 0;
          }
          break;
        }

        default: {
          iprintf("#2 Unknown opcode: 0x%X\n", opcode);
          return 1;
        }
      }
      break;

    case 0x1000: {
      pc = opcode & 0x0FFF;
      iprintf("pc jump to %X\n", pc);
      break;
    }
    
    case 0x2000: {
      sp++;
      pc = opcode & 0x0FFF;
      break;
    }

    case 0x3000: {
      int pos = (opcode & 0x0F00) / 0x0100;
      int val = V[pos];
      int compare = opcode & 0x00FF;
      iprintf("Comparing V%X (0x%X) with 0x%X..", pos, val, compare);
      if (val == compare) {
        iprintf("Same! Skipping.\n");
        pc += 4;
      } else {
        iprintf("Not the same! Continuing as normal.\n");
        pc += 2;
      }
      break;
    }

    case 0x4000: {
      int pos = (opcode & 0x0F00) / 0x0100;
      int val = V[pos];
      int compare = opcode & 0x0FF;
      if (val != compare)
        pc += 4;
      else
        pc += 2;
    }

    case 0x5000: {
      int pos1 = (opcode & 0x0F00) / 0x0100;
      int pos2 = (opcode & 0x00F0) / 0x0010;
      int val1 = V[pos1];
      int val2 = V[pos2];
      if (val1 == val2)
        pc += 4;
      else
        pc += 2;
      break;
    }

    case 0x6000: {
      int val = opcode & 0x00FF;
      int pos = (opcode & 0x0F00) / 0x0100;
      V[pos] = val;
      iprintf("Set V%X to 0x%X\n", pos, val);
      pc += 2;
      break;
    }

    case 0x7000: {
      int pos = (opcode & 0x0F00) / 0x0100;
      int val = V[pos];
      int add = opcode & 0x00FF;
      int newVal = val + add;
      V[pos] = newVal;
      iprintf("V%X += V%X (now 0x%X)\n", pos, add, newVal);
      break;
    }

    case 0x8000: {
      int pos1 = (opcode & 0x0F00) / 0x0100;
      int pos2 = (opcode & 0x00F0) / 0x0010;
      int val1 = V[pos1];
      int val2 = V[pos2];
      switch(opcode & 0x000F) {
        case 0x0000: {
          V[pos1] = val2;
          iprintf("V%X (0x%X) = V%X (0x%X)\n", pos1, val1, pos2, val2);
          break;
        }

        case 0x0001: {
          int newVal = val1 | val2;
          V[pos1] = newVal;
          iprintf("V%X (0x%X) |= V%X (0x%X) (now: 0x%X)\n",
                  pos1, val1, pos2, val2, newVal);
          break;
        }

        case 0x0002: {
          int newVal = val1 & val2;
          V[pos1] = newVal;
          iprintf("V%X (0x%X) &= V%X (0x%X) (now: 0x%X)\n",
                  pos1, val1, pos2, val2, newVal);
          break;
        }

        case 0x0003: {
          int newVal = val1 ^ val2;
          V[pos1] = newVal;
          iprintf("V%X (0x%X) ^= V%X (0x%X) (now: 0x%X)\n",
                  pos1, val1, pos2, val2, newVal);
          break;
        }

        case 0x0004: {
          int newVal = val1 + val2;
          int carry = newVal > 0xFF;
          newVal = newVal % (0xFF + 1);
          V[pos1] = newVal;
          V[15] = carry;
          iprintf("V%X (0x%X) += V%X (0x%X) (now: 0x%X) (carry: %d)\n",
                  pos1, val1, pos2, val2, newVal, carry);
          break;
        }

        case 0x0005: {
          int newVal = val1 - val2;
          int borrow = val1 < val2;
          V[pos1] = newVal;
          V[15] = borrow;
          iprintf("V%X (0x%X) -= V%X (0x%X) (now: 0x%X) (borrow: %d)\n",
                  pos1, val1, pos2, val2, newVal, borrow);
          break;
        }

        case 0x0006: {
          int newVal = val2 >> 1;
          int lsb = val2 & 1;
          V[pos1] = newVal;
          V[15] = lsb;
          iprintf("V%X (0x%X) = V%X (0x%X) >> 1 (now: 0x%X) (old lsb: %X)",
                  pos1, val1, pos2, val2, newVal, lsb);
          break;
        }

        case 0x0007: {
          int newVal = val2 - val1;
          int borrow = val2 < val1;
          V[pos1] = newVal;
          V[15] = borrow;
          iprintf("V%X (0x%X) = V%X - V%X (0x%X) (now: 0x%X) (borrow: %d)\n",
                  pos1, val1, pos1, pos2, val2, newVal, borrow);
          break;
        }

        case 0x000E: {
          int newVal = val2 << 1;
          int msb = 1;
          int temp = val2;
          while (temp >>= 1) msb <<= 1;
          V[pos1] = newVal;
          V[15] = msb;
          iprintf("V%X (0x%X) = V%X (0x%X) << 1 (now: 0x%X) (old msb: %X)\n",
                  pos1, val1, pos2, val2, newVal, msb);
          break;
        }

        default: {
          iprintf("#3 Unknown opcode: 0x%X\n", opcode);
          return 1;
        }
      }
      pc += 2;
      break;
    }

    case 0x9000: {
      int pos1 = (opcode & 0x0F00) / 0x0100;
      int pos2 = (opcode & 0x00F0) / 0x0010;
      int val1 = V[pos1];
      int val2 = V[pos2];
      if (val1 != val2)
        pc += 4;
      else
        pc += 2;
      break;
    }

    case 0xA000: {
      I = opcode & 0x0FFF;
      pc += 2;
      iprintf("I = %X\n", I);
      break;
    }

    case 0xC000: {
      int pos = (opcode & 0x0F00) / 0x0100;
      int mask = (opcode & 0x00FF);
      int val = (rand() * 0xFF) & mask;
      V[pos] = val;
      pc += 2;
      iprintf("V%X = rand & 0x%X (0x%X)\n", pos, mask, val);
      break;
    }
    
    case 0xD000: {
      int pos1 = (opcode & 0x0F00) / 0x0100;
      int pos2 = (opcode & 0x00F0) / 0x0010;
      int x = V[pos1];
      int y = V[pos2];
      int bytes = opcode & 0x000F;

      iprintf("Draw I%X at (0x%X,0x%X) (rows: %X)\n", I, x, y, bytes);

      char sprite[bytes];
      for (int j = 0; j < bytes; j++) {
        iprintf("Draw load: %X %s\n", j,
          std::bitset<8>(memory[I + j]).to_string('0', '1').c_str());
        sprite[j] = memory[I + j];
      }

      for (int i = 0; i < bytes; i++) {
        int row = sprite[i];
        iprintf("Render row: ");
        for (int j = 8; j > 0; j--) {
          int pixel = (row & 1 << j) / 1 << j;
          int px = x + 8 - j;
          int py = y + i;
          int index = py * 64 + px;
          gfxBuf[index] ^= pixel;
          iprintf(pixel ? "1" : "0");
        }
        iprintf("\n");
      }

      pc += 2;
      break;
    }

    case 0xF000: {
      switch(opcode & 0x00FF) {
        case 0x0007: {
          int pos = (opcode & 0x0F00) / 0x0100;
          V[pos] = delay_timer;
          pc += 2;
          iprintf("V%X = timer (0x%X)\n", pos, delay_timer);
          break;
        }

        case 0x0015: {
          int pos = (opcode & 0x0F00) / 0x0100;
          int val = V[pos];
          delay_timer = val;
          pc += 2;
          iprintf("timer = V%X (0x%X)\n", pos, val);
          break;
        }

        case 0x001E: {
          int pos = (opcode & 0x0F00) / 0x0100;
          int val = V[pos];
          int oldI = I;
          I += val;
          iprintf("I (0x%X) += V%X (0x%X) (now: 0x%X)", oldI, pos, val, I);
          pc += 2;
          break;
        }

        case 0x0029: {
          int pos = (opcode & 0x0F00) / 0x0100;
          int val = V[pos];
          int index = 0x050 + val * 5;
          I = index;
          pc += 2;
          iprintf("I = character \"%X\"\n", val);
          break;
        }

        default: {
          iprintf("#4 Unknown opcode: 0x%X\n", opcode);
          return 1;
        }
      }
      break;
    }

    default: {
      iprintf("Unknown opcode: 0x%X\n", opcode);
      return 1;
    }
  }

  // Update timers
  if (delay_timer > 0)
    --delay_timer;

  if (sound_timer > 0) {
    if (sound_timer == 1) {
      printf("BEEP!\n");
    }
    --sound_timer;
  }

  return 0;
}
