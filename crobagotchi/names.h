#pragma ONCE
#include "Arduino.h"

extern const char name1[] PROGMEM = "Crob";
extern const char name2[] PROGMEM = "Crobert";
extern const char name3[] PROGMEM = "Thundercrob";
extern const char name4[] PROGMEM = "CAWWW";
extern const char name5[] PROGMEM = "Mr. Peanuts";
extern const char name6[] PROGMEM = "Ms. Shiny";
// region `text' overflowed by 10 bytes
// extern const char name7[] PROGMEM = "Pecky The Great";
extern const char name8[] PROGMEM = "GIGA CROB";
extern const char name9[] PROGMEM = "CROBOT";
extern const char name10[] PROGMEM = "Geeb";

#define NAMES_LEN 9
const char* crow_names[] = {
    name1, name2, name3, name4, name5, name6, // name7,
    name8, name9, name10
};

char* get_name()
{
    auto idx = random(NAMES_LEN);
    return crow_names[7];
}