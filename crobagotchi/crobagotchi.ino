// Crobagotchi
// Definitely not inspired by any late 90's early 2000's toy

#include "crobgame.h"
#include "crob.h"
#include "ssd1306.h"
#include "nano_gfx.h"
#include "sprites.h"

#include "EEPROM.h"
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

CrobGame game;

// #define DEBUG_WD
// #define DEBUG_SCREEN
// #define DEBUG_LED

// interrupt flags
// these are technically redundant
volatile bool pinInterrupt = false;
volatile bool watchdogInterrupt = false;

bool firstBoot = false;

// this actually counts every 8 seconds
// configured to use the 16mhz internal clock but I think the watchdog timer doesn't care
volatile uint64_t interaction_seconds_counter = 0;

void enableScreen()
{
  ssd1306_128x64_i2c_init();
  ssd1306_setContrast(250);
  ssd1306_normalMode();
  ssd1306_clearScreen();
  ssd1306_setFixedFont(ssd1306xled_font6x8);
}

ISR(WDT_vect)
{
    wdt_reset();
    watchdogInterrupt = true;
    interaction_seconds_counter++;
}

ISR(PCINT0_vect)
{
    wdt_reset();
    pinInterrupt = true;
}

void handle_watchdog_interrupt()
{
  if (MCUSR & _BV(WDRF))  // MCU Status Register, Watchdog Reset Flag
  {
#ifdef DEBUG_WD
      ssd1306_128x64_i2c_init();
      // question: does changing contrast impact the power draw?
      ssd1306_setContrast(255);
      ssd1306_setFixedFont(ssd1306xled_font6x8);
      ssd1306_clearScreen();
      ssd1306_printFixed(0, 24, "WATCH DOG", STYLE_NORMAL);
#endif
      MCUSR = 0;
  }
}

void setup()
{
  cli();
  // CLKPR = 0; // disable clock pre scalar
  // CLKPR = (1 << CLKPCE);
  // CLKPR = 0b0000;
  sei();

  #ifdef DEBUG_LED
  pinMode(0, OUTPUT);
  digitalWrite(0, 1);
  delay(1);
  digitalWrite(0, LOW);
  delay(50);
  #endif
  // clear interrupts
  GIMSK = 0; // General Interrupt Mask Register
  PCMSK = 0; // Pin Change Mask Register
  noInterrupts();
  handle_watchdog_interrupt();

  // we do not use ADC ever
  ADCSRA = 0; // ADC Control Status Register A (reg B does not have an enable bit)
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP); // pb2 pb1

  if (watchdogInterrupt) // watchdog interrupt will reset
  {
    #ifdef DEBUG_LED
      digitalWrite(0, 1);
      delay(5);
      digitalWrite(0, LOW);
      delay(1000);digitalWrite(0, 1);
      delay(5);
      digitalWrite(0, LOW);
      delay(1000);
    #endif
    deepSleep();
  }

  pinInterrupt = false;
  watchdogInterrupt = false;
  interaction_seconds_counter = 0;

enableScreen();
#ifdef DEBUG_SCREEN
  splashScreen();
  ssd1306_printFixed(0, 16, "fresh reboot", STYLE_NORMAL);
  delay(1000);
#endif
  initData();
  firstBoot = true;
}

void screen_off()
{
  ssd1306_normalMode();
  ssd1306_clearScreen();
  ssd1306_displayOff();
  ssd1306_setContrast(255); // set a timer to reduce contrast over time
}

// configures interrupt handlers, shuts off the display, and goes into deep sleep
void deepSleep()
{
#ifdef DEBUG_LED
  digitalWrite(0, 0);
#endif

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  noInterrupts();

  // enable pin change interrupts
  GIMSK |= (1 << PCIE);
  PCMSK |= (1 << PCINT2);
  PCMSK |= (1 << PCINT1);
  
  // watchdog timer
  wdt_reset(); // probably have some redundant calls in here
  MCUSR = 0;
  WDTCR = _BV(WDCE) | _BV(WDE) | _BV(WDIE) | _BV(WDP3) | _BV(WDP0); // 8 sec

  wdt_reset();
  interrupts();

  // sleeps here
  sleep_cpu();
}

void splashScreen()
{
  ssd1306_clearScreen();
  // return;

  for (int i = 0; i < 36; i += 6)
  {
    ssd1306_clearScreen();
    ssd1306_printFixed(30, i, "CROBAGOTCHI", STYLE_NORMAL);
    delay(200);
  }
  ssd1306_printFixed(30, 48, "Ding!", STYLE_NORMAL);

  delay(1000);

  #ifdef DEBUG_SCREEN
    char buf[32] = {0};
    sprintf(buf, "CLKPR %d", CLKPR);
    ssd1306_printFixed(0, 8, buf, STYLE_NORMAL);
    delay(1000);
  #endif
}

void dumpStatus()
{
    ssd1306_clearScreen();
    char buf[32] = {0};
    sprintf(buf, "Age: %d Health: %d", game.crob.age, game.crob.health);
    ssd1306_printFixed(0, 8, buf, STYLE_NORMAL);
    sprintf(buf, "Happy: %d", game.crob.happy);
    ssd1306_printFixed(0, 24, buf, STYLE_NORMAL);

    delay(30000);
}

// init from eeprom, this will resume stored status
void initData()
{
  #ifdef DEBUG_SCREEN
    ssd1306_printFixed(0, 8, "getting data", STYLE_NORMAL);
    delay(1000);
#endif
  auto freshData = game.LoadData();

  freshData = true;

  if (!freshData)
  {
#ifdef DEBUG_SCREEN
    ssd1306_printFixed(0, 8, "init new data", STYLE_NORMAL);
    delay(1000);
#endif

    game.StartNewGame();
    game.SaveData();
  }
  else
  {
    #ifdef DEBUG_SCREEN
    ssd1306_printFixed(0, 8, "using old data", STYLE_NORMAL);
    delay(1000);
    #endif
  }
}

void on_resume()
{
  if (interaction_seconds_counter > 0)
  {
    // decay by counter * 8
#ifdef DEBUG_SCREEN
    char buf[32] = {0};
    sprintf(buf, "Off Time: %d", interaction_seconds_counter);
    ssd1306_printFixed(0, 32, buf, STYLE_NORMAL);
    delay(5000);
#endif
  }
}

void game_loop() // naming consistency, what's that?
{
  // draw status on display
  draw_status();
  
  delay(1000);

  // await button presses (might need to be smart and use interrupts instead of polling, not sure?)

  // dumpStatus();
#ifdef DEBUG_SCREEN
  ssd1306_printFixed(0, 16, "game LOOP", STYLE_NORMAL);
#endif
  delay(1000);
}

void draw_status()
{
  ssd1306_clearScreen();

  // 8 px at the top is for the name and age
  char buf[128 / 5] = {0};
  sprintf(buf, "%s - Age %d", game.crob.name, 123);
  ssd1306_printFixed(0, 0, buf, STYLE_NORMAL);

  // auto pet = ssd1306_createSprite(40, 48, 48, epd_bitmap_test);

  // 4 px margin

  // leaves a 40 px square in the center for the status
  // 128 / 2 = 64
  // 64 - 24 = 40 px from the side
  // nevermind bitmaps have to be multiples of 8
  // ssd1306_drawBitmap(40, 8 / 8, 48, 48, epd_bitmap_test);

// delay(1000);
  ssd1306_clearScreen();
  drawBitMapIntScale(40, 8 / 8, 2, 24, 24, crow_24px);

  // 4 px margin

  // 8 px at the bottom is for the status indicator
  sprintf(buf, "HUNGRY, DIRTY");
  ssd1306_printFixed(8, 56, buf, STYLE_NORMAL);

  delay(3000);

  draw_menu();
}

void drawBitMapIntScale(int x, int y, int scale, int w, int h, const uint8_t *buf)
{
  // ssd1306_write

  // uint8_t i, j;
  //   uint8_t remainder = (ssd1306_lcd.width - x) < w ? (w + x - ssd1306_lcd.width): 0;
  //   w -= remainder;
  //   ssd1306_lcd.set_block(x, y, w);
  //   for(j=(h >> 3); j>0; j--)
  //   {
  //       for(i=w;i>0;i--)
  //       {
  //           ssd1306_lcd.send_pixels1(s_ssd1306_invertByte^pgm_read_byte(buf++));
  //       }
  //       buf += remainder;
  //       ssd1306_lcd.next_page();
  //   }
  //   ssd1306_intf.stop();
}

void draw_menu()
{
  // ssd1306_drawRect(20, 20, 108, 16);
  // ssd1306_printFixed(21, 21, "MENU OPTION", STYLE_NORMAL);
  
  ssd1306_setColor(0);
  ssd1306_fillRect(0, 56, 128, 64);
  ssd1306_negativeMode();
  ssd1306_printFixed(8, 56, "MENU:", STYLE_NORMAL);
  ssd1306_positiveMode();
  ssd1306_printFixed(46, 56, "FEED", STYLE_NORMAL);
}

void loop()
{
  if (pinInterrupt || firstBoot)
  {
    interrupts();
    firstBoot = false;
    // game loop, exits when inactive for x amount of time
    enableScreen();
    splashScreen();

    on_resume();
    game_loop();

    // now that I think about it, saving into eeprom really isn't necessary given
    // that variables are being preserved just fine
    // still is a nice QOL of thing in case the battery dies
    game.SaveData();
    screen_off();
  }

  if (watchdogInterrupt)
  {
    // don't need to do anything special here
    // already incrementing the sleep counter anyways
    // this will be applied on next pin interrupt
  }

  watchdogInterrupt = false;
  pinInterrupt = false;

  screen_off();
  deepSleep(); // pauses here
}
