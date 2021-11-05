#include "crobgame.h"
#include "crob.h"

// put these in different files when I'm dead
void CrobGame::StartNewGame()
{
    this->crob = Crob();
    this->crob.age = 0;
    this->crob.happy = 200;
    this->crob.health = 200;
    this->crob.name = "CROBERT";
}

void CrobGame::LoadData()
{
    ulong buffer[20]; // arbitrary decision
    this->crob.LoadStatus(buffer);
}

void CrobGame::SaveData()
{
    ulong buffer[20];
    this->crob.SaveStatus(buffer);

    // pass this buffer on to the game interface
}

// CrobGame::CrobGame(GameInterface interface)
// {
//     this->interface = interface;
// }

Crob::Crob()
{
    this->name = "UNSET";
    this->health = 0;
    this->happy = 0;
    this->age = 0;
    this->network_bonus_timer = 0;
    this->last_update_time = 0;
}

void Crob::Update(ulong current_time)// maybe do a typedef for the system time if I'm wrong about what the rtc does
{
    if (this->last_update_time == 0)
    {
        // first update, nothing
        this->last_update_time = current_time;
    }

    auto deltatime = current_time - this->last_update_time;

    // this depends on what unit the time is in, so I will wait

    // increment the age for every 6 hours
    // decremnt the health by 50% every 24 hours
    // decrement the happy by 50% every 24 hours, and double this rate if <50% health
    // cut the decrement amounts in half if the network bonus timer is active

    if (this->network_bonus_timer != 0)
    {
        auto delta_net_timer = std::max(this->network_bonus_timer, deltatime);
        this->network_bonus_timer -= delta_net_timer;
    }
}

void Crob::Feed()
{
    // could have just used a const for 255 here...
    // let's see how this turns out when I compile
    auto max_feed = MAX_STAT - this->health;
    auto delta_feed = std::min(max_feed, FEED_AMOUNT);

    this->health += delta_feed;
}

void Crob::Play()
{
    // whatever
    auto max_feed = MAX_STAT - this->happy;
    auto delta_feed = std::min(max_feed, PLAY_AMOUNT);

    this->happy += delta_feed;
}

void Crob::Network(ulong current_time)
{
    this->network_bonus_timer = current_time;
    this->happy = MAX_STAT;
}

void Crob::SaveStatus(ulong *buffer)
{
    // TODO
}

void Crob::LoadStatus(ulong *buffer)
{
    // TODO
}