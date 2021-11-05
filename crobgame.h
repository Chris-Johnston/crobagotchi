#pragma once

#include "crob.h"

// at this point, might as well just consider them both to be the same object
class CrobGame
{
    public:
        Crob crob;
        CrobGame();

        // inits a new crob
        void StartNewGame();

        // loads from the GameInterface the current save data
        void LoadData();

        // saves the current state to the GameInterface
        void SaveData();
    
};