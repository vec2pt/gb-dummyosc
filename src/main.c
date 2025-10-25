#include <gb/gb.h>
#include <gbdk/console.h>
#include <gbdk/font.h>
#include <rand.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Include
#include "pitch.h"
#include "waveforms.h"

// Include assets
#include "assets/tiles.h"

// TODO: Rebuild it
const char hex[] = "0123456789ABCDEF";
void printn(uint16_t n) {
  printf("%c", hex[0x000Fu & (n >> 4u)]);
  printf("%c", hex[0x000Fu & (n)]);
}

// -----------------------------------------------------------------------------
// Variables / Constants
// -----------------------------------------------------------------------------

uint8_t frequency_index = 45, waveform_index = 0;
uint8_t previous_frequency_index = 45; // TODO: Remove it?
bool play = false;

font_t font_norm, font_sel;

uint8_t _wave[16];

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
  set_tile_xy(17, 1, 0x6F);
}

void sound_off(void) {
  NR52_REG = 0x00;
  NR51_REG = 0x00;
  NR50_REG = 0x00;
  set_tile_xy(17, 1, 0x6E);
}

// -----------------------------------------------------------------------------
// main
// TODO: Rebuild it!!!
// -----------------------------------------------------------------------------

void play_sound(void) {
  NR30_REG = 0x00;
  NR32_REG = 0x20;

  for (uint8_t i = 0; i < 16; i++)
    AUD3WAVE[i] = _wave[i];

  NR30_REG |= 0x80;
  NR33_REG = (uint8_t)frequencies[frequency_index];
  NR34_REG = ((uint16_t)frequencies[frequency_index] >> 8) | 0x80;
}

void load_waveform(void) {
  for (uint8_t i = 0; i < 16; i++)
    _wave[i] = waveforms[waveform_index * 16 + i];
}

void osc_draw(uint8_t x, uint8_t y) {
  set_tile_xy(x, y, 0x62);
  for (uint8_t i = 0; i < 16; i++) {
    set_tile_xy(x + 1 + i, y, 0x60);
    set_tile_xy(x + 1 + i, y + 9, 0x60);
  }
  for (uint8_t i = 0; i < 8; i++) {
    set_tile_xy(x, y + 1 + i, 0x61);
    set_tile_xy(x + 17, y + 1 + i, 0x61);
  }
  set_tile_xy(x + 17, y, 0x63);
  set_tile_xy(x, y + 9, 0x64);
  set_tile_xy(x + 17, y + 9, 0x65);
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

void osc_update(uint8_t x, uint8_t y) {
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
      set_tile_xy(x + 1 + i, y + 8 - j, _get_osc_tile(index));
    }
  }
}

void osc_plot(uint8_t x, uint8_t y) {
  gotoxy(x, y);
  for (uint8_t i = 0; i < 16; i++) {
    printn(_wave[i]);
    if (i == 7)
      gotoxy(x, y + 1);
  }
}

void draw(void) { osc_draw(1, 2); }

void update(void) {
  osc_update(1, 2);
  osc_plot(2, 13);

  // TODO
  gotoxy(2, 1);
  printf(pitch_class[frequency_index % 12]);
  printf("%d", octaves[(uint8_t)(frequency_index / 12)]);

  if (!(frequency_index == previous_frequency_index)) {
    play_sound();
  }
}

void check_inputs(void) {
  update_keys();

  // Pitch
  if (key_pressed(J_A)) {
    if (key_ticked(J_RIGHT)) {
      if (frequency_index < 71)
        frequency_index++;
    } else if (key_ticked(J_LEFT)) {
      if (frequency_index > 0)
        frequency_index--;
    } else if (key_ticked(J_UP)) {
      if (frequency_index < 71 - 12)
        frequency_index += 12;
    } else if (key_ticked(J_DOWN)) {
      if (frequency_index > 11)
        frequency_index -= 12;
    }
  }

  // Start / stop play
  else if (key_ticked(J_START)) {
    play = !play;
    if (play) {
      sound_on();
      play_sound();
    } else {
      sound_off();
    }
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
  font_norm = font_load(font_ibm);
  // font_color(0, 3);
  // font_sel = font_load(font_ibm);
  font_set(font_norm);

  set_bkg_data(0x66, tiles_TILE_COUNT, tiles_tiles);
}

void main(void) {
  init();
  draw();

  // Disable sound on startup.
  sound_off();
  // Load the default waveform.
  load_waveform();

  while (1) {
    check_inputs();
    update();
    vsync();
  }
}
