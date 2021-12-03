// Crobagotchi
// Definitely not inspired by any late 90's early 2000's toy

#include "crobgame.h"
#include "crob.h"
// #include "ssd1306.h"
// #include "nano_gfx.h"
#include "sprites.h"

#define BITBANG_SCL PB3
// #define BITBANG_SDA PB4
#define BITBANG_SDA PB4

#include "Tiny4kOLED_bitbang_mod.h"

#include "EEPROM.h"
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

CrobGame game;

#define BUTTON_A 1
#define BUTTON_B 2

// extern const char TextNormal[] PROGMEM = "Normal";
// extern const char TextSleeping[] PROGMEM = "Sleeping";
// extern const char TextHappy[] PROGMEM = "Happy";
// extern const char TextHungry[] PROGMEM = "Hungry";
// extern const char TextSad[] PROGMEM = "Sad";
// extern const char TextDead[] PROGMEM = "Dead";

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
  oled.begin(128, 64, sizeof(tiny4koled_init_128x64br), tiny4koled_init_128x64br);
  oled.clear();
  oled.deactivateScroll();
  oled.disableZoomIn();
  oled.disableFadeOutAndBlinking();

  oled.setFont(FONT6X8);
  oled.on();

  // ssd1306_128x64_i2c_init();
  // ssd1306_setContrast(250);
  // ssd1306_normalMode();
  // ssd1306_clearScreen();
  // ssd1306_setFixedFont(ssd1306xled_font6x8);
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
      // // ssd1306_128x64_i2c_init();
      // // question: does changing contrast impact the power draw?
      // ssd1306_setContrast(255);
      // ssd1306_setFixedFont(ssd1306xled_font6x8);
      // ssd1306_clearScreen();
      // ssd1306_printFixed(0, 24, "WATCH DOG", STYLE_NORMAL);
#endif
      MCUSR = 0;
  }
}

void setup()
{
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
  cli();
  handle_watchdog_interrupt();

  // we do not use ADC ever
  ADCSRA = 0; // ADC Control Status Register A (reg B does not have an enable bit)
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP); // pb2 pb1

  if (watchdogInterrupt) // watchdog interrupt will reset
  {
    #ifdef DEBUG_WD
    enableScreen();
    oled.setCursor(0, 0);
    oled.println("WD in setup()");
    oled.println((int)interaction_seconds_counter);
    delay(200);
    #endif


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

  sei();

enableScreen();
#ifdef DEBUG_SCREEN
  // splashScreen();
  // ssd1306_printFixed(0, 16, "fresh reboot", STYLE_NORMAL);
  oled.print(F("FRESH REBOOT!"));
  delay(1000);
#endif
  initData();
  firstBoot = true;

#ifdef DEBUG_SCREEN
  delay(5000);
#endif
}

void screen_off()
{
  oled.clear();
  oled.off();

  // ssd1306_normalMode();
  // ssd1306_clearScreen();
  // ssd1306_displayOff();
  // ssd1306_setContrast(255); // set a timer to reduce contrast over time
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
  // skip
  return;
  for (int i = 0; i < 4; i += 1)
  {
    oled.clear();
    oled.setCursor(30, i);
    oled.print(F("CROBAGOTCHI"));

    // oled.print("CLKPR ");
    // oled.print(CLKPR);
    // ssd1306_clearScreen();
    // ssd1306_printFixed(30, i, "CROBAGOTCHI", STYLE_NORMAL);
    delay(200);
  }
  // ssd1306_printFixed(30, 48, "Ding!", STYLE_NORMAL);

  // #ifdef DEBUG_SCREEN
  //   char buf[32] = {0};
  //   sprintf(buf, "CLKPR %d", CLKPR);
  //   ssd1306_printFixed(0, 8, buf, STYLE_NORMAL);
  //   delay(1000);
  // #endif
}

void dumpStatus()
{
    oled.clear();
    oled.print(F("Age: "));
    oled.print(game.crob.age);
    oled.print(F(" Health: "));
    oled.print(game.crob.health);
    oled.newLine();
    oled.print(F("Happy: "));
    oled.print(game.crob.happy);
    // ssd1306_clearScreen();
    // char buf[32] = {0};
    // sprintf(buf, "Age: %d Health: %d", game.crob.age, game.crob.health);
    // ssd1306_printFixed(0, 8, buf, STYLE_NORMAL);
    // sprintf(buf, "Happy: %d", game.crob.happy);
    // ssd1306_printFixed(0, 24, buf, STYLE_NORMAL);

    delay(30000);
}

// init from eeprom, this will resume stored status
void initData()
{
  #ifdef DEBUG_SCREEN
  oled.setCursor(0, 0);
    // ssd1306_printFixed(0, 8, "getting data", STYLE_NORMAL);
    oled.println("GETTING DATA");

    auto checksum = EEPROM.read(39);
    oled.print("checksum: ");
  oled.println((int)checksum, 10);
    // delay(1000);
#endif

  auto freshData = game.LoadData();

  // freshData = false;

  if (!freshData)
  {
#ifdef DEBUG_SCREEN
    // ssd1306_printFixed(0, 8, "init new data", STYLE_NORMAL);
    oled.println("INIT NEW DATA");
    delay(1000);
#endif

    game.StartNewGame();
    game.SaveData();
  }
  else
  {
    #ifdef DEBUG_SCREEN
    // ssd1306_printFixed(0, 8, "using old data", STYLE_NORMAL);
    oled.println("USING OLD DATA");
    delay(1000);
    #endif
  }
}

// 1 hour
#define SLEEP_THRESH (60 * 60 / 8)
// #define SLEEP_THRESH 2 // testing

// age increases every 20 min
#define AGE_THRESH (20 * 60 / 8)
// #define AGE_THRESH 2

void on_resume()
{
  if (interaction_seconds_counter > 0)
  {
    // decay by counter * 8
#ifdef DEBUG_SCREEN
    // char buf[32] = {0};
    // sprintf(buf, "Off Time: %d", interaction_seconds_counter);
    // ssd1306_printFixed(0, 32, buf, STYLE_NORMAL);
    oled.print("OFF TIME");
    oled.println((int)interaction_seconds_counter, 10);
    delay(5000);
#endif


    game.crob.isSleeping = interaction_seconds_counter > SLEEP_THRESH;

    // allow overlap, that's funny
    game.crob.age += interaction_seconds_counter / AGE_THRESH;
  }
}

#define IDLE_TIME 15 * 1000
#define DEBOUNCE_TIME 30
#define MENU_DEBOUNCE 100

unsigned long last_debounce_a = 0;
unsigned long last_debounce_b = 0;
bool last_state_a = false;
bool last_state_b = false;

bool get_button(int idx)
{
  bool status = digitalRead(idx);

}

// 0 closed
// 1 - feed
// 2 - play

int menu_option = 0;

#define RESET_HOLD 5 * 1000
#define LONG_RESET_HOLD 9 * 1000

unsigned long reset_hold_time = 0;

void reset_seq()
{
  if ((millis() - reset_hold_time) > RESET_HOLD)
  {
    oled.clear();
    oled.println(F("WARNING!"));
    oled.println(F("Are you sure you wish to reset?"));
    oled.println(F("HOLD only the A\nbutton if so."));

    auto timer = millis();
    bool delet = false;
    while ((millis() - timer) < LONG_RESET_HOLD)
    {
      oled.setCursor(0, 7);
      oled.print((millis() - timer) / 1000);

      // requring held down
      // reminder that buttons are inverted
      delet = !digitalRead(BUTTON_A) && digitalRead(BUTTON_B);
      if (delet)
      {
        oled.print(" YES");
      }
      else
      {
        oled.print(" NO ");
      }
    }

    if (delet)
    {
      oled.clear();
      oled.println(F(":( Bye"));
      delay(2000);

      game.StartNewGame();
      game.SaveData();

      draw_main_menu();
    }
    else
    {
      oled.clear();
      oled.println(F(":)"));
      delay(2000);

      draw_main_menu();
    }
    
  }
}

void game_loop() // naming consistency, what's that?
{
  draw_main_menu();

  bool needsredraw = false;
  auto last_interaction_time = millis();
  // await button presses (might need to be smart and use interrupts instead of polling, not sure?)

  while (millis() < last_interaction_time + IDLE_TIME)
  {
    // draw status on display

    // debounce
    bool a = !digitalRead(BUTTON_A);
    bool b = !digitalRead(BUTTON_B);

    if (a != last_state_a)
    {
      last_debounce_a = millis();
    }
    if (b != last_state_b)
    {
      last_debounce_b = millis();
    }
    last_state_a = a;
    last_state_b = b;

    // reuse the same values
    if ((millis() - last_debounce_a) < DEBOUNCE_TIME)
    {
      a = false;
    }
    if ((millis() - last_debounce_b) < DEBOUNCE_TIME)
    {
      b = false;
    }

    if (a || b)
    {
      last_interaction_time = millis();
    }

    // handle reset
    if (a && b && reset_hold_time == 0)
    {
      reset_hold_time = millis();
      continue;
    }
    else if (a && b && reset_hold_time != 0)
    {
      reset_seq();
      continue;
    }
    else
    {
      reset_hold_time = 0;
    }

    if (b && !a)
    {
      last_debounce_b = millis() + MENU_DEBOUNCE;
      needsredraw = true;
      menu_option += 1;
      menu_option %= 3;

      continue;
    }
    
    if (menu_option)
    {
      oled.setCursor(0, 7);
      oled.invertOutput(true);
      oled.print(F("MENU:"));
      oled.invertOutput(false);
      oled.print(F(" "));

      if (menu_option == 1)
      {
        oled.print("FEED");
      }
      else if (menu_option == 2)
      {
        oled.print("PLAY");
      }
    }
    else if (needsredraw)
    {
      draw_status();
      needsredraw = false;
    }
    

    if (a && !b)
    {
      if (menu_option == 1)
      {
        oled.clear();
        oled.println("FEED SEQ");
        delay(3000);

        game.crob.isSleeping = false;

        game.crob.health += 50;

        draw_main_menu();
        menu_option = 0;
      }

      if (menu_option == 2)
      {
        oled.clear();
        oled.println("PLAY SEQ");
        delay(3000);

        game.crob.isSleeping = false;
        game.crob.happy += 50;

        draw_main_menu();
        menu_option = 0;
      }

      // last_debounce_a = millis() + MENU_DEBOUNCE;

      continue;
    }

    // oled.setCursor(0, 0);

    // if (a)
    //   oled.println("A");
    // if (b)
    //   oled.println("B");
    // if (!a && !b)
    //   oled.println(" ");
  }
  // oled.clear();
  // oled.setCursor(0, 0);
  // oled.println("OFF");
  // delay(3000);
}

void draw_main_menu()
{
  // ssd1306_clearScreen();

  // 8 px at the top is for the name and age
  // char buf[128 / 5] = {0};
  // sprintf(buf, "%s - Age %d", game.crob.name, 123);
  // ssd1306_printFixed(0, 0, buf, STYLE_NORMAL);

  oled.clear();
  oled.setCursor(0, 0);
  oled.print(game.crob.name);
  // oled.print(F(" - Age: "));
  oled.setCursor(128 - 42, 0);
  // 4 
  oled.print(F("Age "));
  // 3
  oled.print(game.crob.age);

  // auto pet = ssd1306_createSprite(40, 48, 48, epd_bitmap_test);

  // 4 px margin

  // leaves a 40 px square in the center for the status
  // 128 / 2 = 64
  // 64 - 24 = 40 px from the side
  // nevermind bitmaps have to be multiples of 8
  // ssd1306_drawBitmap(40, 8 / 8, 48, 48, epd_bitmap_test);
  oled.bitmap(40, 1, 88, 56 / 8, epd_bitmap_test);

  if (game.crob.isSleeping)
  {
    auto z = F("Z");
    oled.setCursor(90 + random(5), 4);
    oled.print(z);

    oled.setCursor(95 + random(5), 3);
    oled.print(z);

    oled.setCursor(100 + random(5), 2);
    oled.print(z);
  }

// delay(1000);
  // ssd1306_clearScreen();
  // drawBitMapIntScale(40, 8 / 8, 2, 24, 24, crow_24px);

  // 4 px margin

  // 8 px at the bottom is for the status indicator
  // sprintf(buf, "HUNGRY, DIRTY");
  // ssd1306_printFixed(8, 56, buf, STYLE_NORMAL);
  draw_status();

  // delay(3000);

  // draw_menu(0);
}

void draw_status()
{
  oled.setCursor(0, 7);
  // oled.print("HUNGRY, DIRTY");
  oled.print(F("Status: "));
  
  switch (game.crob.GetStatus())
  {
    case Status::Sad:
      oled.print(F("SAD"));
      break;
    case Status::Sleeping:
      oled.print(F("SLEEPING"));
      break;
    case Status::Happy:
      oled.print(F("HAPPY"));
      break;
    case Status::Dead:
      oled.print(F("DEAD"));
      break;
    case Status::Normal:
    default:
      oled.print(F("NORMAL"));
      break;
  }
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

void draw_menu(int cursorIdx)
{
  const uint8_t empty[] = {0x0};
    for (int i = 0; i < 128; i++)
    {
      for(int y = 0; y < 8; y++)
        oled.bitmap(i, y, i + 1, y, empty);
      delay(50);
    }

  oled.setCursor(8, 1);
  oled.println(F("MENU:"));

  if (cursorIdx == 0)
    {
      oled.print(F("> "));
    }
  oled.println(F("FEED"));
  oled.println(F("PLAY"));

  // ssd1306_drawRect(20, 20, 108, 16);
  // ssd1306_printFixed(21, 21, "MENU OPTION", STYLE_NORMAL);
  
  // ssd1306_setColor(0);
  // ssd1306_fillRect(0, 56, 128, 64);
  // ssd1306_negativeMode();
  // ssd1306_printFixed(8, 56, "MENU:", STYLE_NORMAL);
  // ssd1306_positiveMode();
  // ssd1306_printFixed(46, 56, "FEED", STYLE_NORMAL);
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

    interaction_seconds_counter = 0;
  }

  if (watchdogInterrupt)
  {
    // don't need to do anything special here
    // already incrementing the sleep counter anyways
    // this will be applied on next pin interrupt

    #ifdef DEBUG_WD
    enableScreen();
    oled.setCursor(0, 0);
    oled.println("WD in loop()");
    oled.println((int)interaction_seconds_counter);
    delay(200);
    #endif
  }

  watchdogInterrupt = false;
  pinInterrupt = false;

  screen_off();
  deepSleep(); // pauses here
}
