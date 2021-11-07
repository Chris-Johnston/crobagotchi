// Crobagotchi
// Definitely not inspired by any late 90's early 2000's toy

#include "crobgame.h"
#include "crob.h"
#include "ssd1306.h"
#include "nano_gfx.h"
#include "sprites.h"

#include "EEPROM.h"

// #include <Wire.h>


// #define SDA 0
// #define SCL 2

// tinywire library has some neat features
// but is glitchy for me
// #include <TinyWireM.h>
// #include <Tiny4kOLED_bitbang.h>
// #include <Tiny4kOLED.h>

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>


CrobGame game;
volatile bool timerInterrupt = false;
volatile bool pinInterrupt = false;
volatile bool watchdogInterrupt = false;

// this actually counts every 8 seconds
volatile uint64_t interaction_seconds_counter = 0;

void setup()
{
    ssd1306_128x64_i2c_init();
  ssd1306_setContrast(255);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  // ssd1306_clearScreen();

  splashScreen();

//   ssd1306_sendData(0xD5);
//   ssd1306_sendData(0xf0);
  
  // ssd1306_printFixed(0, 24, "WATCH DOG", STYLE_NORMAL);
  
  


  GIMSK = 0;
  PCMSK = 0;
  cli();

  // we do not use ADC ever... unless I want low voltage detection?
  ADCSRA = 0;

  if (MCUSR & _BV(WDRF))
  {
      ssd1306_128x64_i2c_init();
  ssd1306_setContrast(255);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_clearScreen();
  ssd1306_printFixed(0, 24, "WATCH DOG", STYLE_NORMAL);


      MCUSR = 0;
      updateSleepTime();

      delay(180000);
  }

  if (timerInterrupt)
  {
       updateSleepTime();
       return;
  }
  else
  {
      EEPROM.write(100, 0);
  }
  

  // this init will only be necessary for pin interrupts, timer interrupts will skip most of this
  ssd1306_128x64_i2c_init();
  ssd1306_setContrast(255);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_clearScreen();

//   if (pinInterrupt || timerInterrupt)
//   {
//       splashScreen();
//   }

  if (pinInterrupt)
  {
      sleepTime();
      delay(50000);
  }

    watchdogInterrupt = false;
  timerInterrupt = false;
  pinInterrupt = false;

  // TinyWireM.begin();

//   // oled.begin();
//   oled.begin(128, 64, sizeof(tiny4koled_init_128x64br), tiny4koled_init_128x64br);
//   // oled.begin(128, 64, sizeof(tiny4koled_init_128x64b), tiny4koled_init_128x64b);
//   oled.setFont(FONT6X8);
//   oled.invertOutput(false);
//   oled.setInternalIref(true);
//   oled.setContrast(255);
//   oled.clearToEOL();
//   oled.clear();
//   oled.bitmap(0, 0, 128, 64, epd_bitmap_leg_dithered);

//   oled.switchFrame();
//   oled.clear();

//   oled.fill(0);

  // oled.blink(200);
  // oled.on();
  // oled.setInternalIref(true);
  // oled.on();
  // oled.switchFrame();
//   oled.setEntireDisplayOn(true);
//     oled.on();

//   while (true)
//   {
//     oled.clear();

//      oled.setCursor(0, 1);
//      oled.print(F("ms: "));
//      oled.print(millis());
//     // oled.on();

//     // oled.clear();
//     oled.switchFrame();
//     // oled.bitmap(0, 0, 128, 64, epd_bitmap_leg_dithered);

//     // delay(1000);
//     // delay(100);
//     // oled.on();
//     delay(100);
//     // oled.off();
//   }

  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP); // pb2 pb1


  ssd1306_printFixed(0, 8, "getting data", STYLE_NORMAL);
  initData();
  delay(1000);
  dumpStatus();
  delay(50000);

  splashScreen();
}



void deepSleep()
{
  ssd1306_normalMode();
  ssd1306_clearScreen();
  // ssd1306_displayOff();
  ssd1306_setContrast(255); // set a timer to reduce contrast over time

  cli();
  // sei();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  // set_sleep_mode(SLEEP_MODE_IDLE);
  // set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  sleep_enable();

  noInterrupts();

  // enable pin change interrupts
  // GIMSK = 0b100000;
  GIMSK |= (1 << PCIE);
  PCMSK |= (1 << PCINT2);
  PCMSK |= (1 << PCINT1);
  // PCMSK = 0b110;

  // watchdog timer
  wdt_reset();
  MCUSR = 0;
  WDTCR = _BV(WDCE) | _BV(WDE) | _BV(WDIE) | _BV(WDP3) | _BV(WDP0);

  wdt_reset();
  

// TIMER
// TCCR1 = 0;
// TCNT1 = 0;
// GTCCR = _BV(PSR1);
//   // enable timer interrupts
//   TCCR1 |= (1 << CTC1);
//   // https://embeddedthoughts.com/2016/06/06/attiny85-introduction-to-pin-change-and-timer-interrupts/
//   // TCCR1 |= (1 << CS13) | (1 << CS12) | (1 << CS11);

//   // http://www.arduinoslovakia.eu/application/timer-calculator
//   OCR1C = 243;
//   OCR1A = OCR1C;
//   TCCR1 |= (1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10);
//   // TCCR1 = _BV(CTC1) | _BV(CS13) | _BV(CS12) | _BV(CS11) | _BV(CS10);
//   // TCCR1 = _BV(CS01) ;
//   TIMSK |= (1 << OCIE1A);

    sei();
  interrupts();

  sleep_cpu(); // this shuts down more fully??
  // sleep_cpu();
  // sleep_mode();

  cli();
}

ISR(WDT_vect)
{
    wdt_disable();
    watchdogInterrupt = true;
    timerInterrupt = true;
    interaction_seconds_counter++;
}

// ISR(TIMER1_COMPA_vect)
// {
//     timerInterrupt = true;
//     interaction_seconds_counter++;
//     // pinInterrupt = false;
// }

ISR(PCINT0_vect)
{
    // timerInterrupt = false;
    pinInterrupt = true;
    // cli();
}

// ISR(PCINT1_vect)
// {
//     timerInterrupt = false;
//     pinInterrupt = true;
//     // cli();
// }

// ISR(PCINT2_vect)
// {
//     timerInterrupt = false;
//     pinInterrupt = true;
//     // cli();
// }



// ISR(PCINT1_vect)
// {
//     isPinChange = true;
// }

// ISR(PCINT2_vect)
// {
//     isPinChange = true;
// }

void splashScreen()
{
  ssd1306_drawBitmap(0, 0, 128, 64, epd_bitmap_leg_dithered);
  delay(100);
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

void sleepTime()
{
    auto slept = EEPROM.read(100);

    ssd1306_clearScreen();
    char buf[32] = {0};
    sprintf(buf, "Sleeping: %d", slept);
    ssd1306_printFixed(0, 8, buf, STYLE_NORMAL);
}

void updateSleepTime()
{
    auto slept = EEPROM.read(100);
    slept += 1;
    if (slept == 255)
    {
        slept = 0;
    }
    EEPROM.write(100, slept);
}

// init from eeprom, this will resume stored status
void initData()
{
  auto freshData = game.LoadData();

  if (!freshData)
  {
    ssd1306_printFixed(0, 8, "init new data", STYLE_NORMAL);

    game.StartNewGame();
  }
  else
  {
    ssd1306_printFixed(0, 8, "using old data", STYLE_NORMAL);
  }

}


void loop()
{


  game.SaveData();
  watchdogInterrupt = false;
  pinInterrupt = false;
  timerInterrupt = false;

    ssd1306_displayOff();
  deepSleep();

  cli();
  // setup();

  if (pinInterrupt)
  {
      ssd1306_128x64_i2c_init();
        ssd1306_setFixedFont(ssd1306xled_font6x8);
        ssd1306_clearScreen();

        ssd1306_printFixed(0, 8, "waking up", STYLE_NORMAL);
      ssd1306_printFixed(0, 16, "PIN INTERRUPT", STYLE_NORMAL);
      sleepTime();
      delay(150000);
  }

  if (timerInterrupt)
  {
      updateSleepTime();

      return;

        // ssd1306_128x64_i2c_init();
        ssd1306_displayOn();
        ssd1306_setFixedFont(ssd1306xled_font6x8);
        ssd1306_clearScreen();

        if (watchdogInterrupt || MCUSR & _BV(WDRF))
    {
        ssd1306_printFixed(0, 24, "watchdog!!!!", STYLE_NORMAL);
    }

        ssd1306_printFixed(0, 8, "TIME", STYLE_NORMAL);
        char buf[32] = {0};
    sprintf(buf, "T: %d", interaction_seconds_counter);
    ssd1306_printFixed(0, 16, buf, STYLE_NORMAL);
        // ssd1306_clearScreen();
        // delay(11000);
        // ssd1306_displayOff();

      // deepSleep();
  }

  // delay(150000);

  // delay(50000);
}
