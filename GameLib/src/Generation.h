#pragma once
#include <BitPounce.h>
#include <random>
#include "BoundsInt.h"

#define WALL_0 16611798124123392899
#define WALL_1 3771104731740525241

void AddPlacingItemsCallback(std::function<bool(glm::ivec2 pos, std::mt19937& rng, BoundsInt room)> callback);
BitPounce::Entity GenDungeon(BitPounce::Ref<BitPounce::Scene> scene, uint32_t seed);

