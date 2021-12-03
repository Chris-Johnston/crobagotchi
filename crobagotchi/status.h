#pragma once
// #include <limits> I dont need to include this just for a 255 const

// this is either the simulator, or the real hardware
// could do this in 2 phases,
// have game simulator which could just be the console for all I care
// and then the display simulator which handles menu and drawing stuff
// class GameInterface
// {
//     public:
//         CrobGame game;
//         virtual void DrawStatus();
// };


enum Status
{
    // 25% - 75% for both dimensions
    Normal,
    // been 'a while' since the last interaction with user
    Sleeping,
    // happy > 90%
    Happy,
    // health < 20%
    Hungry,
    // happy < 20%, health > 20%
    Sad,
    // RIP
    Dead,
};

