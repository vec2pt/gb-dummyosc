#include <gb/gb.h>
#include <gbdk/console.h>
#include <gbdk/font.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// TODO:
// User interface for selecting waveform shape.
// Waveform editor (AUD3WAVE).

// Include
#include "pitch.h"
#include "waveforms.h"

// Include assets
#include "assets/logo.h"
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

#define VERSION "v0.1.1"

#define OSC_X 1
#define OSC_Y 2

// Current and previous indexes should be different at initialization.
uint8_t frequency_index = 45, frequency_previous_index = 44;
uint8_t waveform_index = 0, waveform_previous_index = 1;
uint16_t period_value;
bool play = false;

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
// -----------------------------------------------------------------------------

void play_isr(void) {
  NR30_REG = 0;

  for (uint8_t i = 0; i < 16; i++)
    AUD3WAVE[i] = waveforms[waveform_index * 16 + i];

  NR30_REG = 0x80;
  NR31_REG = 0xFE;
  NR32_REG = 0x20;

  period_value = frequencies[frequency_index];
  NR33_REG = period_value & 0xFF;
  NR34_REG = 0xC0 | (period_value >> 8);
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

  gotoxy(0, 17);
  printf(VERSION);
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
    fS = waveforms[waveform_index * 16 + i] >> 4;
    eS = waveforms[waveform_index * 16 + i] & 0x0F;
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
    printn(waveforms[waveform_index * 16 + i]);
    if (i == 7)
      gotoxy(x, y + 1);
  }
}

void update(void) {
  if (waveform_index != waveform_previous_index) {
    osc_update(OSC_X, OSC_Y);
    osc_plot(2, 13);
  }

  if (frequency_index != frequency_previous_index) {
    gotoxy(2, 1);
    printf(pitch_class[frequency_index % 12]);
    printf("%d", octaves[(uint8_t)(frequency_index / 12)]);
  }
}

void check_inputs(void) {
  update_keys();

  // Waveform
  waveform_previous_index = waveform_index;
  if (!key_pressed(J_A)) {
    if (key_ticked(J_UP)) {
      if (waveform_index == WAVEFORMS_COUNT - 1)
        waveform_index = 0;
      else
        waveform_index++;
    } else if (key_ticked(J_DOWN)) {
      if (waveform_index == 0)
        waveform_index = WAVEFORMS_COUNT - 1;
      else
        waveform_index--;
    }
  }

  // Pitch
  frequency_previous_index = frequency_index;
  if (key_pressed(J_A)) {
    if (key_ticked(J_RIGHT)) {
      if (frequency_index < FREQUENCIES_COUNT - 1)
        frequency_index++;
    } else if (key_ticked(J_LEFT)) {
      if (frequency_index > 0)
        frequency_index--;
    } else if (key_ticked(J_UP)) {
      if (frequency_index < FREQUENCIES_COUNT - 12)
        frequency_index += 12;
    } else if (key_ticked(J_DOWN)) {
      if (frequency_index > 11)
        frequency_index -= 12;
    }
  }

  // Start / stop play
  if (key_ticked(J_START)) {
    play = !play;
    (play) ? sound_on() : sound_off();
  }
}

void show_logo(void) {
  gotoxy(6, 4);
  printf("DummyOSC");

  // Logo
  set_bkg_tiles(5, 5, logo_WIDTH / 8, logo_HEIGHT / 8, logo_map);

  gotoxy(5, 13);
  printf("by  VEC2PT");

  vsync();
  delay(2200);
  cls();
}

void setup(void) {
  __critical {
    TMA_REG = 0xC0, TAC_REG = 0x07;
    add_TIM(play_isr);
    set_interrupts(VBL_IFLAG | TIM_IFLAG);
  }

  // Font
  font_init();
  font_load(font_ibm);

  // Tiles
  set_bkg_data(0x66, tiles_TILE_COUNT, tiles_tiles);
  set_bkg_data(0x66 + tiles_TILE_COUNT, logo_TILE_COUNT, logo_tiles);

  show_logo();
}

void main(void) {
  setup();
  osc_draw(OSC_X, OSC_Y);

  // Disable sound on startup.
  sound_off();

  while (1) {
    check_inputs();
    update();
    vsync();
  }
}
