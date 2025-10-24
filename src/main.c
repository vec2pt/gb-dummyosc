#include <gb/gb.h>
#include <gbdk/console.h>
#include <gbdk/font.h>
#include <rand.h>
#include <stdint.h>
#include <stdio.h>

// Include
#include "pitch.h"

// Include assets
#include "assets/tiles.h"

// TODO: waveforms.h?
uint8_t _wave[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                     0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

// TODO: Rebuild it
const char hex[] = "0123456789ABCDEF";
void printn(uint16_t n) {
  printf("%c", hex[0x000Fu & (n >> 4u)]);
  printf("%c", hex[0x000Fu & (n)]);
}

// -----------------------------------------------------------------------------
// Variables / Constants
// -----------------------------------------------------------------------------

uint8_t frequency_index = 12;
uint8_t previous_frequency_index = 12; // TODO: Remove it?

// -----------------------------------------------------------------------------
// Inputs
// -----------------------------------------------------------------------------

uint8_t previous_keys = 0, keys = 0;

void update_keys(void) {
  previous_keys = keys;
  keys = joypad();
}

uint8_t key_pressed(uint8_t aKey) { return keys & aKey; }
uint8_t key_ticked(uint8_t aKey) {
  return (keys & aKey) && !(previous_keys & aKey);
}
uint8_t key_released(uint8_t aKey) {
  return !(keys & aKey) && (previous_keys & aKey);
}

// -----------------------------------------------------------------------------
// Sound controle
// -----------------------------------------------------------------------------

void sound_on(void) {
  NR52_REG = 0x80;
  NR51_REG = 0x44;
  NR50_REG = 0x77;
}

void sound_off(void) {
  NR52_REG = 0x00;
  NR51_REG = 0x00;
  NR50_REG = 0x00;
}

// -----------------------------------------------------------------------------
// main
// TODO: Rebuild it!!!
// -----------------------------------------------------------------------------

void play_sound(void) {
  // TODO: Check regs
  NR30_REG = 0x00;
  // NR32_REG = 0x40;
  // NR31_REG = 0xFE;
  NR32_REG = 0x20;

  for (uint8_t i = 0; i < 16; i++)
    AUD3WAVE[i] = _wave[i];

  NR30_REG |= 0x80;
  NR33_REG = (uint8_t)frequencies[frequency_index];
  NR34_REG = ((uint16_t)frequencies[frequency_index] >> 8) | 0x80;
}

uint8_t _get_osc_tile(uint8_t aIndex) {
  if (aIndex == 0b0010) {
    return 0x66;
  } else if (aIndex == 0b1000) {
    return 0x67;
  } else if (aIndex == 0b0001) {
    return 0x68;
  } else if (aIndex == 0b0100) {
    return 0x69;
  } else if (aIndex == 0b0011) {
    return 0x6A;
  } else if (aIndex == 0b1100) {
    return 0x6B;
  } else if (aIndex == 0b0110) {
    return 0x6C;
  } else if (aIndex == 0b1001) {
    return 0x6D;
  } else {
    return 0x00;
  }
}

void draw_osc(void) {
  uint8_t fS, eS;
  uint8_t index;
  for (uint8_t i = 0; i < 16; i++) {
    fS = _wave[i] >> 4;
    eS = _wave[i] & 0x0F;
    for (int8_t j = 0; j < 8; j++) {
      index = 0b0000;
      if (fS == j * 2) {
        index |= 0b1000;
      } else if (fS == (j * 2 + 1)) {
        index |= 0b0010;
      }
      if (eS == j * 2) {
        index |= 0b0100;
      } else if (eS == (j * 2 + 1)) {
        index |= 0b0001;
      }
      set_tile_xy(2 + i, 9 - j, _get_osc_tile(index));
    }
  }
}

void draw(void) {
  set_tile_xy(1, 1, 0x62);
  for (uint8_t i = 0; i < 16; i++) {
    set_tile_xy(2 + i, 1, 0x60);
    set_tile_xy(2 + i, 10, 0x60);
  }
  for (uint8_t i = 0; i < 8; i++) {
    set_tile_xy(1, 2 + i, 0x61);
    set_tile_xy(18, 2 + i, 0x61);
  }
  set_tile_xy(18, 1, 0x63);
  set_tile_xy(1, 10, 0x64);
  set_tile_xy(18, 10, 0x65);
  draw_osc();

  // TODO
  gotoxy(2, 11);
  printf(pitch_class[frequency_index % 12]);
  printf("%d  ", octaves[(uint8_t)(frequency_index / 12)]);
  printf("%d", frequencies[frequency_index]);
}

void update(void) {
  draw_osc();

  // TODO
  gotoxy(2, 11);
  printf(pitch_class[frequency_index % 12]);
  printf("%d  ", octaves[(uint8_t)(frequency_index / 12)]);
  printf("%d", frequencies[frequency_index]);

  if (!(frequency_index == previous_frequency_index)) {
    play_sound();
  }
}

void check_inputs(void) {
  update_keys();

  // if (key_pressed(J_A)) {
  //   printf("J_A");
  // }
  // if (key_ticked(J_B)) {
  //   printf("J_B");
  // }
  // if (key_released(J_B)) {
  //   printf("R");
  // }
  if (key_ticked(J_UP)) {
    previous_frequency_index = frequency_index;
    if (frequency_index < 71)
      frequency_index++;
  }
  if (key_ticked(J_DOWN)) {
    previous_frequency_index = frequency_index;
    if (frequency_index > 0)
      frequency_index--;
  }
  if (key_ticked(J_RIGHT)) {
    previous_frequency_index = frequency_index;
    if (frequency_index < 71 - 12)
      frequency_index += 12;
  }
  if (key_ticked(J_LEFT)) {
    previous_frequency_index = frequency_index;
    if (frequency_index > 11)
      frequency_index -= 12;
  }
}

void init(void) {
  DISPLAY_ON;
  HIDE_WIN;
  SHOW_BKG;
  HIDE_SPRITES;
  // SHOW_SPRITES;
  font_init();
  // font_load(font_min);
  font_load(font_ibm);
  set_bkg_data(0x66, tiles_TILE_COUNT, tiles_tiles);

  // Sound
  NR52_REG = 0x80;
  NR51_REG = 0x44;
  NR50_REG = 0x77;

  play_sound();
}

void main(void) {
  init();
  draw();

  while (1) {
    check_inputs();
    update();
  }
}
