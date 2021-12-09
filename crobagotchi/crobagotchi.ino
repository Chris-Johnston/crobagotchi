// Crobagotchi
// Definitely not inspired by any late 90's early 2000's toy

#include "crobgame.h"
#include "crob.h"
// #include "ssd1306.h"
// #include "nano_gfx.h"
#include "sprites.h"

#define BITBANG_SCL PB3
#define BITBANG_SDA PB4

#include "Tiny4kOLED_bitbang_mod.h"
#include "font6x8caps_mod.h"

#include "EEPROM.h"
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

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
  // oled.deactivateScroll();
  // oled.disableZoomIn();
  // oled.disableFadeOutAndBlinking();
  oled.setFont(FONT6X8CAPSMOD);
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
  pinInterrupt = false;
  firstBoot = false;
  interaction_seconds_counter++;
}

ISR(PCINT0_vect)
{
  pinInterrupt = true;
  watchdogInterrupt = false;
  firstBoot = false;
}

EMPTY_INTERRUPT(ADC_vect);
EMPTY_INTERRUPT(ANA_COMP_vect);
EMPTY_INTERRUPT(EE_RDY_vect);
EMPTY_INTERRUPT(INT0_vect);
EMPTY_INTERRUPT(TIM0_COMPA_vect);
EMPTY_INTERRUPT(TIM1_COMPA_vect);
EMPTY_INTERRUPT(TIM1_OVF_vect);
EMPTY_INTERRUPT(USI_OVF_vect);
EMPTY_INTERRUPT(USI_START_vect);
EMPTY_INTERRUPT(BADISR_vect);


// ISR(BADISR_vect)
// {
//   firstBoot = false;
// }>

void handle_watchdog_interrupt()
{
#ifdef DEBUG_WD
  if (MCUSR & _BV(WDRF)) // MCU Status Register, Watchdog Reset Flag
  {
    // // ssd1306_128x64_i2c_init();
    // // question: does changing contrast impact the power draw?
    // ssd1306_setContrast(255);
    // ssd1306_setFixedFont(ssd1306xled_font6x8);
    // ssd1306_clearScreen();
    // ssd1306_printFixed(0, 24, "WATCH DOG", STYLE_NORMAL);
    // MCUSR = 0;
  }
#endif
}

void clear_interrupt()
{
  cli();
  // disable pin change
  GIMSK = 0; // General Interrupt Mask Register

  // clear the interrupt flag Register
  GIFR = 0;

  // do not have to clear the watchdog interrupts, because it should operate silently
  // and should reset in case of crash as well
  // I think this is casuing crashes after it goes to sleep and comes back

  // disable all WD stuff
    // WDTCR = _BV(WDCE) | _BV(WDE);
    wdt_reset();
    WDTCR |= _BV(WDCE); //  | _BV(WDE);
    WDTCR = 0;

    sei();
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
#ifdef DEBUG_WD
  handle_watchdog_interrupt();
#endif

  // we do not use ADC ever
  ADCSRA = 0; // ADC Control Status Register A (reg B does not have an enable bit)
  PRR &= 0b1110; // disable ADC
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP); // pb2 pb1

  // unreachable code
  //   if (watchdogInterrupt) // watchdog interrupt will reset
  //   {
  // #ifdef DEBUG_WD
  //     enableScreen();
  //     oled.setCursor(0, 0);
  //     oled.println("WD in setup()");
  //     oled.println((int)interaction_seconds_counter);
  //     delay(200);
  // #endif

  // #ifdef DEBUG_LED
  //     digitalWrite(0, 1);
  //     delay(5);
  //     digitalWrite(0, LOW);
  //     delay(1000);
  //     digitalWrite(0, 1);
  //     delay(5);
  //     digitalWrite(0, LOW);
  //     delay(1000);
  // #endif
  //     deepSleep();
  //   }

  pinInterrupt = false;
  watchdogInterrupt = false;
  interaction_seconds_counter = 0;

#ifdef DEBUG_SCREEN
  enableScreen();
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

  // this added 38 bytes, ouch!
  // 8k of flash is HARD
  // if (MCUSR & _BV(BORF)) // brown out reset flag
  // {
  //   enableScreen();
  //   oled.println(F("BAT"));
  //   delay(255);
  // }

  if (MCUSR)
  {
    enableScreen();
    oled.println(MCUSR);
  }

  MCUSR = 0;
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

  GIFR = 0;

  // enable pin change interrupts
  GIMSK = (1 << PCIE);
  PCMSK |= (1 << PCINT2) | (1 << PCINT1);

  // watchdog timer
  // WDCE = Change enable
  // WDE = Enable WD
  // WDIE = Watchdog timeout interrupt enable (cleared each time)
  wdt_reset();
  WDTCR = _BV(WDCE) | _BV(WDE) | _BV(WDIE) | _BV(WDP3) | _BV(WDP0); // 8 sec


// power_adc_disable();
// power_timer0_disable();
// power_timer1_disable();
// power_usi_disable();
power_all_disable();
  // clear pin interrupts

  // wdt_reset(); // not necessary here
  interrupts();


  // this fixes the screen going away and immediately coming back
  // I think re-enabling interrupts immediately calls one of the ISRs
  watchdogInterrupt = false;
  pinInterrupt = false;

  // sleeps here
  sleep_cpu();
  sleep_disable();

  power_timer0_enable();

  // wake up
}

void splashScreen()
{
#ifdef OLDSPLASH
  // skip
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
#endif
  // ssd1306_printFixed(30, 48, "Ding!", STYLE_NORMAL);

  // #ifdef DEBUG_SCREEN
  //   char buf[32] = {0};
  //   sprintf(buf, "CLKPR %d", CLKPR);
  //   ssd1306_printFixed(0, 8, buf, STYLE_NORMAL);
  //   delay(1000);
  // #endif
}

// void dumpStatus()
// {
//     oled.clear();
//     oled.print(F("Age: "));
//     oled.print(game.crob.age);
//     oled.print(F(" Health: "));
//     oled.print(game.crob.health);
//     oled.newLine();
//     oled.print(F("Happy: "));
//     oled.print(game.crob.happy);
//     // ssd1306_clearScreen();
//     // char buf[32] = {0};
//     // sprintf(buf, "Age: %d Health: %d", game.crob.age, game.crob.health);
//     // ssd1306_printFixed(0, 8, buf, STYLE_NORMAL);
//     // sprintf(buf, "Happy: %d", game.crob.happy);
//     // ssd1306_printFixed(0, 24, buf, STYLE_NORMAL);

//     delay(30000);
// }

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

// age increases every 120 min
#define AGE_THRESH (120 * 60 / 8)
// #define AGE_THRESH 2

// happy should decay by 75 every day
#define HAPPY_DECAY 144
// food should decay by 100 every day
#define FEED_DECAY 108

void on_resume()
{
  if (interaction_seconds_counter > 0 && !game.crob.getIsDead())
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

    // game.crob.health = game.crob.health - (interaction_seconds_counter / FEED_DECAY);
    game.crob.setHealth(game.crob.health - interaction_seconds_counter / FEED_DECAY);

    // game.crob.happy = game.crob.happy - (interaction_seconds_counter / HAPPY_DECAY);
    game.crob.setHappy(game.crob.happy - interaction_seconds_counter / HAPPY_DECAY);

    // allow overflow, that's funny
    game.crob.age += interaction_seconds_counter / AGE_THRESH;
  }
}

#define IDLE_TIME (15 * 1000)
#define DEBOUNCE_TIME (30)
#define MENU_DEBOUNCE (500)

unsigned long last_debounce_a = 0;
unsigned long last_debounce_b = 0;
unsigned long menu_debounce = 0;
bool last_state_a = false;
bool last_state_b = false;
bool last_menu = false;

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
    // oled.println(F("WARNING!"));
    oled.println(F("HOLD A TO RESET"));

    auto timer = millis();
    bool delet = false;
    while ((millis() - timer) < LONG_RESET_HOLD)
    {
      clear_bot_line();
      oled.print((millis() - timer) / 1000);
      oled.setCursor(18, 7);

      // requring held down
      // reminder that buttons are inverted
      delet = !digitalRead(BUTTON_A) && digitalRead(BUTTON_B);
      if (delet)
      {
        oled.print(F("YES"));
      }
      else
      {
        oled.print(F("NO"));
      }

      delay(300);
    }

    if (delet)
    {
      oled.clear();
      oled.println(F(":( BYE"));
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

void draw_fry()
{
  // fill the screen

  // without loops this uses way more
  for (int x = 0; x < 120; x += 24)
    for (int y = 0; y < 64; y += 32)
      oled.bitmap(x, y / 8, x + 24, y / 8 + 3, epd_bitmap_fry);

  delay(3000);
}

void feed_seq()
{
  oled.clear();

  draw_fry();

  // oled.println(F("FEED SEQ"));
  // delay(3000);

  game.crob.isSleeping = false;
  // game.crob.health = min(255, game.crob.health + 50);
  game.crob.setHealth(game.crob.health + 120);
}

void play_seq()
{
  // play seq
        oled.clear();
        // I only have like, 70 bytes left
        oled.bitmap(0, 0, 48, 6, epd_bitmap_test);
        delay(300);
        oled.clear();
        oled.bitmap(40, 2, 88, 8, epd_bitmap_test);
        delay(300);
        oled.clear();
        oled.bitmap(80, 0, 128, 6, epd_bitmap_test);
        delay(300);

        game.crob.isSleeping = false;
        // game.crob.happy = min(255, game.crob.happy + 50);
        game.crob.setHappy(game.crob.happy + 120);
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
      last_interaction_time = millis();
      continue;
    }
    else
    {
      reset_hold_time = 0;
    }

    bool menu = b && !a;
    // debounce the menu
    if ((millis() - menu_debounce) < MENU_DEBOUNCE)
    {
      menu = false;
    }
    
    // move through menu
    else if (menu)
    {

      if (!game.crob.getIsDead())
      {
        needsredraw = true;
        menu_option += 1;
        menu_option %= 3;
      }
      else
      {
        menu_option = 0;
      }

      menu_debounce = millis();
    }

    last_menu = menu;


    if (needsredraw)
    {
      clear_bot_line();
      if (menu_option)
      {
        // oled.invertOutput(true);
        oled.print(F("MENU:"));
        // oled.invertOutput(false);
        oled.setCursor(40, 7);

        if (menu_option == 1)
        {
          oled.print(F("FEED"));
        }
        else if (menu_option == 2)
        {
          oled.print(F("PLAY"));
        }
      }
      else
      {
        draw_status();
      }
      needsredraw = false;
    }

    if (a && !b)
    {
      if (menu_option == 0)
      {
        continue;
      }
      else if (menu_option == 1)
      {
        feed_seq();
      }
      else if (menu_option == 2)
      {
        play_seq();
      }

      draw_main_menu();
      menu_option = 0;
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
  oled.print(F("AGE "));
  // 3
  oled.print(game.crob.age);

  // auto pet = ssd1306_createSprite(40, 48, 48, epd_bitmap_test);

  // 4 px margin

  // leaves a 40 px square in the center for the status
  // 128 / 2 = 64
  // 64 - 24 = 40 px from the side
  // nevermind bitmaps have to be multiples of 8
  // ssd1306_drawBitmap(40, 8 / 8, 48, 48, epd_bitmap_test);
  if (!game.crob.getIsDead())
  {
    oled.bitmap(40, 1, 88, 56 / 8, epd_bitmap_test);

    if (game.crob.isSleeping || game.crob.GetStatus() == Status::Happy)
    {
      auto z = F("Z");
      if (game.crob.GetStatus() == Status::Happy)
      {
        z = F("^");
      }
      oled.setCursor(90, 4);
      oled.print(z);

      oled.setCursor(95, 3);
      oled.print(z);

      oled.setCursor(100, 2);
      oled.print(z);
    }
  }
  else
  {
    oled.setCursor(58, 3);
    oled.print(F("RIP"));
  }

  // delay(1000);
  // ssd1306_clearScreen();
  // drawBitMapIntScale(40, 8 / 8, 2, 24, 24, crow_24px);

  // 4 px margin

  // 8 px at the bottom is for the status indicator
  // sprintf(buf, "HUNGRY, DIRTY");
  // ssd1306_printFixed(8, 56, buf, STYLE_NORMAL);
  clear_bot_line();
  draw_status();

  // delay(3000);

  // draw_menu(0);
}

void clear_bot_line()
{
  oled.setCursor(0, 7);
  oled.clearToEOL();
}

void draw_status()
{
  // clear_bot_line();
  // oled.print("HUNGRY, DIRTY");
  // oled.print(F("STATUS: "));

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

// void draw_menu(int cursorIdx)
// {
//   oled.setCursor(8, 1);
//   oled.clearToEOL();
//   oled.println(F("MENU:"));

//   if (cursorIdx == 0)
//   {
//     oled.print(F("> "));
//   }
//   oled.println(F("FEED"));
//   oled.println(F("PLAY"));

//   // ssd1306_drawRect(20, 20, 108, 16);
//   // ssd1306_printFixed(21, 21, "MENU OPTION", STYLE_NORMAL);

//   // ssd1306_setColor(0);
//   // ssd1306_fillRect(0, 56, 128, 64);
//   // ssd1306_negativeMode();
//   // ssd1306_printFixed(8, 56, "MENU:", STYLE_NORMAL);
//   // ssd1306_positiveMode();
//   // ssd1306_printFixed(46, 56, "FEED", STYLE_NORMAL);
// }

void loop()
{
  if (pinInterrupt || firstBoot)
  {
    clear_interrupt();
    enableScreen();
    if (firstBoot)
    {
      splashScreen();
    }

    firstBoot = false;
    // game loop, exits when inactive for x amount of time
    splashScreen();

    on_resume();

    // now that I think about it, saving into eeprom really isn't necessary given
    // that variables are being preserved just fine
    // still is a nice QOL of thing in case the battery dies or if it crashes
    game.SaveData();

    game_loop();

    interaction_seconds_counter = 0;
  }

#ifdef DEBUG_WD
  if (watchdogInterrupt)
  {
    // don't need to do anything special here
    // already incrementing the sleep counter anyways
    // this will be applied on next pin interrupt
    enableScreen();
    oled.setCursor(0, 0);
    oled.println("WD in loop()");
    oled.println((int)interaction_seconds_counter);
    delay(200);
  }
#endif

  screen_off();
  deepSleep(); // pauses here
}
