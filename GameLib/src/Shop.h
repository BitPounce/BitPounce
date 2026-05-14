#include <BitPounce.h>

struct Item
{
	BitPounce::AssetHandle texHandle;
	uint64_t ID;
	std::string name;

	// in £
	uint64_t Money = 0;

	float radius = 1;

	// 10 / fireRate
	uint8_t fireRate = 1;
};

void Shop_Load(std::vector<Item> items);
std::vector<Item> Shop_GetPlayerItems();
Item Shop_GetItem();
void Shop_Save();
void Shop_AddMoney(uint64_t Money);
bool Shop_Buy(uint64_t ID);
void Shop_ImGuiDraw();