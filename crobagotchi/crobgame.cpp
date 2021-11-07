#include "crobgame.h"
#include "crob.h"
#include "Arduino.h"
#include <EEPROM.h>

// put these in different files when I'm dead
void CrobGame::StartNewGame()
{
    this->crob = Crob();
    this->crob.age = 1;
    this->crob.happy = 2;
    this->crob.health = 3;
    // char n[32] = "test";
    // this->crob.name = n;
}

bool CrobGame::LoadData()
{
    uint8_t buffer[SAVEDATA_LEN];
    for (auto i = 0; i < SAVEDATA_LEN; i++)
    {
        buffer[i] = EEPROM.read(i);
    }

    bool existingData = this->crob.LoadStatus(buffer);

    if (!existingData)
    {
        // init fresh data
    }

    return existingData;

    // buffer to eeprom
}

void CrobGame::SaveData()
{
    uint8_t buffer[SAVEDATA_LEN];
    this->crob.SaveStatus(buffer);

    for (auto i = 0; i < SAVEDATA_LEN; i++)
    {
        EEPROM.write(i, buffer[i]);
    }
}

CrobGame::CrobGame()
{
    this->crob = Crob();
}

// CrobGame::CrobGame(GameInterface interface)
// {
//     this->interface = interface;
// }

Crob::Crob()
{
    // char n[32] = "name";
    // this->name = n;
    this->health = 0;
    this->happy = 0;
    this->age = 0;
    // this->network_bonus_timer = 0;
    // this->last_update_time = 0;
    this->sleepCycles = 0;
}

// void Crob::Update(ulong current_time)// maybe do a typedef for the system time if I'm wrong about what the rtc does
// {
//     if (this->last_update_time == 0)
//     {
//         // first update, nothing
//         this->last_update_time = current_time;
//     }

//     auto deltatime = current_time - this->last_update_time;

//     // this depends on what unit the time is in, so I will wait

//     // increment the age for every 6 hours
//     // decremnt the health by 50% every 24 hours
//     // decrement the happy by 50% every 24 hours, and double this rate if <50% health
//     // cut the decrement amounts in half if the network bonus timer is active

//     if (this->network_bonus_timer != 0)
//     {
//         auto delta_net_timer = max(this->network_bonus_timer, deltatime);
//         this->network_bonus_timer -= delta_net_timer;
//     }
// }

void Crob::Feed()
{
    // could have just used a const for 255 here...
    // let's see how this turns out when I compile
    auto max_feed = MAX_STAT - this->health;
    auto delta_feed = min(max_feed, FEED_AMOUNT);

    this->health += delta_feed;
}

void Crob::Play()
{
    // whatever
    auto max_feed = MAX_STAT - this->happy;
    auto delta_feed = min(max_feed, PLAY_AMOUNT);

    this->happy += delta_feed;
}

// save status to the buffer
void Crob::SaveStatus(uint8_t *buffer)
{
    uint8_t chksum = 0;

    // for (auto i = 0; i < NAME_LEN; i++)
    // {
    //     buffer[i] = this->name[i];
    //     chksum += buffer[i];
    // }

    buffer[33] = this->health;
    buffer[34] = this->happy;
    buffer[35] = this->age;
    buffer[36] = this->sleepCycles;

    chksum += this->health;
    chksum += this->happy;
    chksum += this->age;
    chksum += this->sleepCycles;

    // prevent values of 0 that look unset
    if (chksum == 0)
    {
        chksum = 0xff;
    }

    buffer[39] = chksum;
}

// load status from the buffer
bool Crob::LoadStatus(uint8_t *buffer)
{
    uint8_t chksum = buffer[39];

    if (chksum == 0)
    {
        // unset data
        return false;
    }

    uint8_t actualSum = 0;

    // for (auto i = 0; i < NAME_LEN; i++)
    // {
    //     this->name[i] = buffer[i];
    //     actualSum += buffer[i];
    // }

    this->health = buffer[33];
    this->happy = buffer[34];
    this->age = buffer[35];
    this->sleepCycles = buffer[36];

    actualSum += this->health;
    actualSum += this->happy;
    actualSum += this->age;
    actualSum += this->sleepCycles;

    return chksum == actualSum;
}