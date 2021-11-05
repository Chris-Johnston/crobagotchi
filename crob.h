#pragma once
#include <string>

#include "status.h"

#define FEED_AMOUNT 128
#define PLAY_AMOUNT 128
#define MAX_STAT 255


// represents the players pet
class Crob
{
    public:
        std::string name;
        // these are calculated values each time update is called
        // the delta time between last_update_time and current determines
        // how these values will degrade
        // 0 = dead, 255 = max
        uint8_t health;
        uint8_t happy;

        // 1 per 6 hours?
        uint8_t age;

        // these should exist under crob because they will be part of the buffer

        // when crow is networked, timer is set to 2 days
        // this will cut the degrade rates in half
        // this is calculated by subtracting the update time
        ulong network_bonus_timer;

        // the time of the last update
        ulong last_update_time;

        Crob();

        // updates the state and determines the calculated properties
        void Update(ulong current_time);

        // gets the status to display
        Status GetStatus();

        // feeds
        void Feed();

        // plays, pets
        void Play();

        // enable network bonus
        void Network(ulong current_time);

        // saves status into the given buffer
        void SaveStatus(ulong *buffer);

        // loads status from a buffer
        void LoadStatus(ulong *buffer);
};

// g++ *.cpp `pkg-config --cflags --libs glib-2.0 gtkmm-3.0`