#pragma ONCE
#include "Arduino.h"


// extern const char name1[] PROGMEM = "Crob";
// extern const char name2[] PROGMEM = "Crobert";
// extern const char name3[] PROGMEM = "Thundercrob";
// extern const char name4[] PROGMEM = "CAWWW";
// extern const char name5[] PROGMEM = "Mr. Peanuts";
// extern const char name6[] PROGMEM = "Ms. Shiny";
// // region `text' overflowed by 10 bytes
// // extern const char name7[] PROGMEM = "Pecky The Great";
// extern const char name8[] PROGMEM = "GIGA CROB";
// extern const char name9[] PROGMEM = "CROBOT";
// extern const char name10[] PROGMEM = "Geeb";

#define NAMES_LEN 9

String get_name()
{
    srandom(millis());
    auto idx = random(NAMES_LEN);
    switch (idx)
    {
        case 0: return F("Crob");
        case 1: return F("Crobert");
    case 2: return F("Thundercrob");
    case 3: return F("CAWW");
    case 4: return F("Mr. Peanuts");
    case 5: return F("Ms. Shiny");
    case 6: return F("Pecky");
    case 7: return F("GIGA CROB");
    case 8: return F("CROBOT");
    case 9: return F("Geeb");
    }
    
    // return crow_names[idx];
}