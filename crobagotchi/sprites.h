#pragma ONCE

// #include "ssd1306_hal/io.h"
#include <stdint.h>
#include <Arduino.h>

// #define POOBY
#ifdef POOBY
#define CHARACTER_SPRITE epd_bitmap_poob
#else
#define CHARACTER_SPRITE epd_bitmap_test
#endif
// extern const uint8_t epd_bitmap_leg_dithered [] PROGMEM;

extern const uint8_t epd_bitmap_test [] PROGMEM;

// extern const uint8_t crow_24px [] PROGMEM;

extern const uint8_t epd_bitmap_fry [] PROGMEM;

extern const uint8_t epd_bitmap_poob [] PROGMEM;