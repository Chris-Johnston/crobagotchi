#pragma once

typedef unsigned long ulong;
// typedef unsigned byte uint8_t;
#include "stdint.h"

#include "status.h"

#define FEED_AMOUNT 128
#define PLAY_AMOUNT 128
#define MAX_STAT 255

#define SAVEDATA_LEN 40
#define NAME_LEN 32

#define SLEEPING_IDLE_CYCLES 50

// represents the players pet
class Crob
{
    public:
        char name[NAME_LEN] = {0};
        // char name*;
        // these are calculated values each time update is called
        // the delta time between last_update_time and current determines
        // how these values will degrade
        // 0 = dead, 255 = max
        int16_t health; // HACK clamp these to bytes but use a signed 16 because I AM LAZY
        int16_t happy;

        void setHealth(int16_t health)
        {
            this->health = clamp(health);
        }

        void setHappy(int16_t happy)
        {
            this->happy = clamp(happy);
        }

        int16_t clamp(int16_t v)
        {
            return v < 0 ? 0 : (v > 255 ? 255 : v);
        }

        // 1 per 6 hours?
        uint8_t age; // this is fine tho

        // these should exist under crob because they will be part of the buffer

        // // when crow is networked, timer is set to 2 days
        // // this will cut the degrade rates in half
        // // this is calculated by subtracting the update time
        // ulong network_bonus_timer;

        // // the time of the last update
        // ulong last_update_time;

        // this represents the number of sleep cycles since the time
        // the device was last interrupted by a pin change
        // if the device is interrupted by sleep, then increment this value up to 255
        // uint8_t sleepCycles;
        bool isSleeping = false;

        Crob();

        // updates the state and determines the calculated properties
        // void Update(ulong current_time);

        // gets the status to display
        Status GetStatus();

        // feeds
        void Feed();

        // plays, pets
        void Play();

        // enable network bonus
        // void Network(ulong current_time);

        // saves status into the given buffer
        void SaveStatus(uint8_t *buffer);

        // loads status from a buffer
        bool LoadStatus(uint8_t *buffer);

        bool getIsDead() { return health == 0; }
};

// g++ *.cpp `pkg-config --cflags --libs glib-2.0 gtkmm-3.0`