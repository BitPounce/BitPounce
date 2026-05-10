#pragma once

#include <BitPounce.h>

struct Player
{
    BitPounce::Entity tilemap;
    uint8_t frime = 0;
    uint8_t fps = 4;
    BitPounce::Timer Timer;
};

