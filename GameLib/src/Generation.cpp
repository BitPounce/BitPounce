#include "Generation.h"
#include <random>
#include <unordered_set>
#include <queue>
#include "BoundsInt.h"

static std::array<glm::ivec2, 4> s_DirList = {
	glm::ivec2(0, 1),
	glm::ivec2(1, 0),
	glm::ivec2(0, -1),
	glm::ivec2(-1, 0)
};
static int iterations = (32);
static constexpr int CORRIDOR_WIDTH = 1;

static void SplitVertically(int minWidth, std::queue<BoundsInt>& roomsQueue, BoundsInt room, std::mt19937& rng)
{
	std::uniform_int_distribution<int> Dist(1, room.size.x);
    auto xSplit = Dist(rng);
    BoundsInt room1 = BoundsInt(room.min(), glm::ivec3(xSplit, room.size.y, room.size.z));
    BoundsInt room2 = BoundsInt(glm::ivec3(room.min().x + xSplit, room.min().y, room.min().z),
        glm::ivec3(room.size.x - xSplit, room.size.y, room.size.z));
    roomsQueue.push(room1);
    roomsQueue.push(room2);
}

static void SplitHorizontally(int minHeight, std::queue<BoundsInt>& roomsQueue, BoundsInt room, std::mt19937& rng)
{
	std::uniform_int_distribution<int> Dist(1, room.size.y);
    auto ySplit = Dist(rng);
    BoundsInt room1 = BoundsInt(room.min(), glm::ivec3(room.size.x, ySplit, room.size.z));
    BoundsInt room2 = BoundsInt(glm::ivec3(room.min().x, room.min().y + ySplit, room.min().z),
        glm::ivec3(room.size.x, room.size.y - ySplit, room.size.z));
    roomsQueue.push(room1);
    roomsQueue.push(room2);
}

static std::vector<BoundsInt> BinarySpacePartitioning(BoundsInt spaceToSplit, int minWidth, int minHeight, std::mt19937& rng)
{
	std::queue<BoundsInt> roomsQueue{};
	std::vector<BoundsInt> roomsList{};
	roomsQueue.push(spaceToSplit);
	std::uniform_int_distribution<int> distribution{0, 10}; 
	while(roomsQueue.size() > 0)
	{
		auto room = roomsQueue.front();
		roomsQueue.pop();
		if(room.size.y >= minHeight && room.size.x >= minWidth)
		{
			if(distribution(rng) < 5)
			{
				if(room.size.y >= minHeight * 2)
                {
                    SplitHorizontally(minHeight, roomsQueue, room, rng);
                }else if(room.size.x >= minWidth * 2)
                {
                    SplitVertically(minWidth, roomsQueue, room, rng);
                }else if(room.size.x >= minWidth && room.size.y >= minHeight)
                {
                    roomsList.emplace_back(room);
                }
			}
			else
			{
				if (room.size.x >= minWidth * 2)
                {
                    SplitVertically(minWidth, roomsQueue, room, rng);
                }
                else if (room.size.y >= minHeight * 2)
                {
                    SplitHorizontally(minHeight, roomsQueue, room, rng);
                }
                else if (room.size.x >= minWidth && room.size.y >= minHeight)
                {
                    roomsList.emplace_back(room);
                }
			}
		}
	}

	return roomsList;
}

static glm::ivec2 GetRandomElement(const std::unordered_set<glm::ivec2>& set, std::mt19937& rng)
{
	std::uniform_int_distribution<size_t> dist(0, set.size() - 1);
	size_t index = dist(rng);
	auto it = set.begin();
	std::advance(it, index);
	return *it;
}

// Perform a single random walk of given length, starting from startPos
static std::unordered_set<glm::ivec2> RandomWalk(glm::ivec2 startPos, int walkLength, std::mt19937& rng)
{
	std::uniform_int_distribution<int> dirDist(0, s_DirList.size() - 1);
	std::unordered_set<glm::ivec2> path;
	path.emplace(startPos);
	glm::ivec2 prevPos = startPos;

	for (int i = 0; i < walkLength; ++i) {
		int dirIndex = dirDist(rng);
		glm::ivec2 newPos = prevPos + s_DirList[dirIndex];
		path.emplace(newPos);
		prevPos = newPos;
	}
	return path;
}

static std::unordered_set<glm::ivec2 > FindWallsInDir(std::unordered_set<glm::ivec2 > floorPos, std::array<glm::ivec2,4> dirList)
{
	std::unordered_set<glm::ivec2 > wallPos{};
	for (auto&& pos : floorPos)
	{
		for(auto&& dir : dirList)
		{
			auto neighbourPos = pos + dir;
			if(!floorPos.contains(neighbourPos)) wallPos.emplace(neighbourPos);
		}
	}
	return wallPos;
}

static void CreateWalls(std::unordered_set<glm::ivec2 > floorPos, BitPounce::TilemapComponent& tilemap, BitPounce::Ref<BitPounce::Scene> scene)
{
	std::unordered_set<glm::ivec2> basicWallPos = FindWallsInDir(floorPos, s_DirList);
	for(auto&& pos : basicWallPos)
	{
		BitPounce::Renderer2D::TileQuad tile{};
		tile.pos = glm::vec3((float)pos.x, (float)pos.y, 0.0f);
		tile.tex = WALL_0;
		//BitPounce::Entity haha = scene->CreateEntity();
		//haha.GetTransform().Translation = tile.pos;
		//auto&& rb = haha.AddComponent<BitPounce::Rigidbody2DComponent>();
		//rb.Type = BitPounce::Rigidbody2DComponent::BodyType::Static;
		//haha.AddComponent<BitPounce::BoxCollider2DComponent>();
		

		tilemap.AddTile(tile);
	}
}




// Run multiple random walks, each time picking a new starting point from the accumulated tiles
static std::unordered_set<glm::ivec2> RunRandomWalk(glm::ivec2 startingPoint,uint32_t seed)
{
	std::mt19937 rng(seed);
	std::uniform_int_distribution<int> walkLengthDist(10, 100);

	glm::ivec2 currPos = startingPoint;
	std::unordered_set<glm::ivec2> floorPos;

	for (int i = 0; i < iterations; ++i) {
		int length = walkLengthDist(rng);
		auto path = RandomWalk(currPos, length, rng);
		floorPos.merge(path);                     // C++17: merge inserts unique elements

		// Choose a new random starting position from the current set
		if (!floorPos.empty()) {
			currPos = GetRandomElement(floorPos, rng);
		}
	}
	return floorPos;
}

static std::vector<glm::ivec2> RandomWalkCorridor(glm::ivec2 startPos, int corridorLength, std::mt19937& rng, std::unordered_set<glm::ivec2>& potentialRoomPositions)
{
    std::uniform_int_distribution<int> dirDist(0, s_DirList.size() - 1);
    auto dir = s_DirList[dirDist(rng)];               // direction of the corridor (forward)
    
    // Perpendicular directions for widening
    glm::ivec2 perp1(-dir.y, dir.x);   // rotate 90° left
    glm::ivec2 perp2(dir.y, -dir.x);   // rotate 90° right

    std::vector<glm::ivec2> corridor;
    glm::ivec2 currentPos = startPos;

    for (int step = 0; step < corridorLength; ++step)
    {
        // For each step, add a thick cross‑section
        // Center cell
        corridor.push_back(currentPos);
        // Offsets perpendicular to the direction
        for (int w = 1; w <= CORRIDOR_WIDTH; ++w)
        {
            corridor.push_back(currentPos + perp1 * w);
            corridor.push_back(currentPos + perp2 * w);
        }
        // Move forward for the next step
        currentPos += dir;
    }
    return corridor;
}

static bool CreateRoomsAtDeadEnd(std::vector<glm::ivec2> deadEnds, std::unordered_set<glm::ivec2>& roomFloors, uint32_t seed)
{
	bool ret = false;
	for(auto pos : roomFloors)
	{
		if(roomFloors.contains(pos) == false)
		{
			auto room = RunRandomWalk(pos, seed);
			roomFloors.merge(room);
			ret = true;
		}
	}
	return ret;
}

static std::vector<glm::ivec2> FindAllDeadEnds(std::unordered_set<glm::ivec2> floorPositions)
{
	std::vector<glm::ivec2> deadEnds{};
	for(auto pos : floorPositions)
	{
		int neighboursCount = 0;
		for (size_t i = 0; i < 4; i++)
		{
			if(floorPositions.contains(pos + s_DirList[i]))
				neighboursCount++;
		}
		if (neighboursCount == 1)
            deadEnds.emplace_back(pos);
	}
	return deadEnds;
}

static std::unordered_set<glm::ivec2> CreateRooms(std::unordered_set<glm::ivec2> potentialRoomPositions, uint32_t seed)
{
    std::unordered_set<glm::ivec2> roomPositions;

    // Copy to vector for indexing/shuffling
    std::vector<glm::ivec2> potential(
        potentialRoomPositions.begin(),
        potentialRoomPositions.end()
    );

    size_t potentialCount = potential.size();
    int roomToCreateCount = static_cast<int>(std::round(potentialCount * 0.9f));

    // Random engine
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::shuffle(potential.begin(), potential.end(), gen);

    if (roomToCreateCount > (int)potential.size())
        roomToCreateCount = (int)potential.size();

    for (int i = 0; i < roomToCreateCount; i++)
    {
        const glm::ivec2& roomPosition = potential[i];

        auto roomFloor = RunRandomWalk(roomPosition, seed);

        roomPositions.insert(roomFloor.begin(), roomFloor.end());
    }

    return roomPositions;
}

static std::unordered_set<glm::ivec2> RunCorridorFirstGeneration(std::mt19937 rng, uint32_t seed)
{
	std::uniform_int_distribution<int> walkLengthDist(10, 100);
	auto currentPosition = glm::i32vec2(0);
	int corridorCount = walkLengthDist(rng);
	
	std::unordered_set<glm::ivec2> floorPositions;
	std::unordered_set<glm::ivec2> potentialRoomPositions;

    for (int i = 0; i < corridorCount; i++)
    {
		int corridorLength = walkLengthDist(rng);
        auto corridor = RandomWalkCorridor(currentPosition, corridorLength, rng, potentialRoomPositions);
		potentialRoomPositions.emplace(currentPosition);
        currentPosition = corridor[corridor.size() - 1];
		potentialRoomPositions.emplace(currentPosition);
        floorPositions.insert(corridor.begin(), corridor.end());
    }

	std::unordered_set<glm::ivec2> roomPositions = CreateRooms(potentialRoomPositions, seed);
	
	std::vector<glm::ivec2> deadEnds = FindAllDeadEnds(floorPositions);

	bool ret = CreateRoomsAtDeadEnd(deadEnds, roomPositions, seed);
	while (ret)
	{
		deadEnds = FindAllDeadEnds(floorPositions);

		ret = CreateRoomsAtDeadEnd(deadEnds, roomPositions, seed);
	}
	
    
	floorPositions.merge(roomPositions);

	return floorPositions;
}

std::unordered_set<glm::ivec2> CreateSimpleRooms(std::vector<BoundsInt> roomsList)
{
	std::unordered_set<glm::ivec2> floor{};
	for(auto&& room : roomsList)
	{
		for (int col = 1; col < room.size.x - 1; col++)
		{
			for (int row = 1; row < room.size.y - 1; row++)
			{
				glm::ivec2 pos = (glm::ivec2)room.min() + glm::ivec2(col, row);
				floor.emplace(pos);
			}
		}
	}
	return floor;
}

static glm::ivec2 FindClosestPointTo(glm::ivec2 currentRoomCenter, std::vector<glm::ivec2> roomCenters)
{
	glm::ivec2 closest = glm::ivec2(0);
	float distance = 3.4028235E+38;
	for(auto&& pos : roomCenters)
	{
		float currentDistance = glm::distance((glm::vec2)pos, (glm::vec2)currentRoomCenter);
		if(currentDistance < distance)
		{
			distance = currentDistance;
            closest = pos;
		}
	}
	return closest;
}

static std::unordered_set<glm::ivec2> CreateCorridor(glm::ivec2 currentRoomCenter, glm::ivec2 destination)
{
	std::unordered_set<glm::ivec2> corridor{};
	auto position = currentRoomCenter;
	corridor.emplace(position);
	while (position.y != destination.y)
	{
		if(destination.y > position.y)
		{
			position += glm::ivec2(0, 1);
		}
		if(destination.y < position.y)
		{
			position += glm::ivec2(0, -1);
		}
		corridor.emplace(position);
	}
	while (position.x != destination.x)
	{
		if(destination.x > position.x)
		{
			position += glm::ivec2(1, 0);
		}
		if(destination.x < position.x)
		{
			position += glm::ivec2(-1, 0);
		}
		corridor.emplace(position);
	}

	return corridor;
}
static std::unordered_set<glm::ivec2> ConnectRooms(std::vector<glm::ivec2> roomCenters, std::mt19937& rng)
{
	std::unordered_set<glm::ivec2> corridors{};
	std::uniform_int_distribution<int> dis{0, static_cast<int>(roomCenters.size() - 1)};
	auto currentRoomCenter = roomCenters[dis(rng)];
	while (roomCenters.size() > 0)
	{
		glm::ivec2 closest = FindClosestPointTo(currentRoomCenter, roomCenters);
		roomCenters.erase(std::remove(roomCenters.begin(), roomCenters.end(), closest), roomCenters.end());
		std::unordered_set<glm::ivec2> newCorridor = CreateCorridor(currentRoomCenter, closest);
		currentRoomCenter = closest;
		corridors.merge(newCorridor);
	}
	return corridors;
}

static std::unordered_set<glm::ivec2> BSP_CreateRooms(std::mt19937& rng)
{
	auto roomsList = BinarySpacePartitioning(BoundsInt(glm::ivec3(0,0,0), glm::ivec3(40,40,0)), 8,8, rng);
	std::unordered_set<glm::ivec2> floor{};
	floor = CreateSimpleRooms(roomsList);

	std::vector<glm::ivec2> roomCenters{};
	roomCenters.reserve(roomsList.size());
	for(auto room : roomsList)
	{
		roomCenters.emplace_back((glm::ivec2)room.center());
	}
	std::unordered_set<glm::ivec2> corridors = ConnectRooms(roomCenters, rng);
	floor.merge(corridors);

	return floor;
}

BitPounce::Entity GenDungeon(BitPounce::Ref<BitPounce::Scene> scene)
{
	uint32_t seed = 435435;
	std::mt19937 rng(seed);
	BitPounce::Entity ent = scene->CreateEntity();
	BitPounce::TilemapComponent& tilemapComponent = ent.AddComponent<BitPounce::TilemapComponent>();
	BP_CORE_INFO("{}", (size_t)&tilemapComponent);

	std::unordered_set<glm::ivec2 > data = /*RunCorridorFirstGeneration(rng, seed)*/ BSP_CreateRooms(rng);
	tilemapComponent.Reserve(data.size());

	for (auto&& pos : data) {
		//BP_CORE_INFO("X: {0}, Y: {1}", pos.x, pos.y);
		BitPounce::Renderer2D::TileQuad tile{};
		tile.pos = glm::vec3((float)pos.x, (float)pos.y, 0.0f);
		tile.tex = WALL_1;
		tilemapComponent.AddTile(tile);


	}
	CreateWalls(data, tilemapComponent, scene);

	return ent;
}