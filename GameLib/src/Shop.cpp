#include "Shop.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <imgui.h>

struct Shop
{
	std::vector<Item> items;
	std::vector<uint64_t> player_items;
	uint64_t money;
	uint64_t currItem = 0;
};

static Shop s_Shop;

void Shop_Save();

void Shop_Load(std::vector<Item> items) 
{
	s_Shop = {};
	s_Shop.items = items;

	Item item = {};
	item.fireRate = 9;
	item.ID = 0;
	item.Money = 0;
	item.radius = 1.5f;
	item.texHandle = 13101426457884025154;
	s_Shop.items.push_back(item);

	if(!std::filesystem::exists("shop.json"))
	{
		s_Shop.money = 60;
		s_Shop.player_items.push_back(0);
		s_Shop.currItem = 0;

		Shop_Save();

		// hehehe
		goto loaded_json;
	}

	{
		BitPounce::BufferBase buffer = BitPounce::FileSystem::LoadFile("shop.json");

		std::string jsonStr((char*)buffer.Data, buffer.Size);

		nlohmann::json json = nlohmann::json::parse(jsonStr);

		s_Shop.money = json["money"].get<uint64_t>();
		s_Shop.currItem = json["currItem"].get<uint64_t>();

		for (auto& playerItem : json["playerItems"])
		{
			s_Shop.player_items.push_back(playerItem.get<uint64_t>());
		}
	}

	goto loaded_json;

loaded_json:
	return;
}

std::vector<Item> Shop_GetPlayerItems()
{
	std::vector<Item> result;

	for (uint64_t id : s_Shop.player_items)
	{
		for (const auto& item : s_Shop.items)
		{
			if (item.ID == id)
			{
				result.push_back(item);
				break;
			}
		}
	}

	return result;
}

Item Shop_GetItem()
{
    for (const auto& item : s_Shop.items)
	{
		if(item.ID == s_Shop.currItem)
		{
			return item;
		}
	}

	return {};
}

void Shop_Save() 
{
	nlohmann::json json;

	json["money"] = s_Shop.money;
	json["currItem"] = s_Shop.currItem;
	json["playerItems"] = s_Shop.player_items;

	std::ofstream file("shop.json");
	if (file.is_open())
	{
		file << json.dump(1, '\t');
		file.close();
	}
}

void Shop_AddMoney(uint64_t Money) 
{
	s_Shop.money += Money;
	Shop_Save();
}

bool Shop_Buy(uint64_t ID) 
{
	// already owned check
	for (auto owned : s_Shop.player_items)
	{
		if (owned == ID)
			return false;
	}

	// find item
	for (const auto& item : s_Shop.items)
	{
		if (item.ID == ID)
		{
			if (s_Shop.money < item.Money)
				return false;

			s_Shop.money -= item.Money;
			s_Shop.player_items.push_back(ID);

			Shop_Save();
			return true;
		}
	}

	return false;
}

void Shop_ImGuiDraw() 
{
	ImGui::Begin("Shop");
	static float padding = 16.0f;
	static float thumbnailSize = 100.0f;

	float cellSize = thumbnailSize + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;
	ImGui::Text((std::string("You have £") + std::to_string(s_Shop.money)).c_str());
	ImGui::Columns(columnCount, 0, false);

	for (auto&& item : s_Shop.items)
	{
		ImGui::ImageButton(std::to_string(item.texHandle.operator uint64_t()).c_str(), (void*)BitPounce::AssetManager::GetAsset<BitPounce::Texture2D>(item.texHandle)->GetRendererID(), ImVec2(100,100), { 0, 1 }, { 1, 0 });
		bool playerHasItem = false;
		for (auto&& item2 : s_Shop.player_items)
		{
			if(item2 == item.ID)
			{
				playerHasItem = true;
			}
		}
		ImGui::PushID((std::string("fgjgghjfjhgjjkyfhjyb") + std::to_string(item.texHandle.operator uint64_t())).c_str());
		if(playerHasItem && ImGui::Button("E"))
		{
			s_Shop.currItem = item.ID;
		}
		else if (!playerHasItem && ImGui::Button((std::string("Buy £") + std::to_string(item.Money) + std::string(" ") + item.name).c_str()))
		{
			Shop_Buy(item.ID);
		}
		ImGui::NextColumn();
		ImGui::PopID();
		
	}
	ImGui::Columns(1);

	ImGui::End();
}
