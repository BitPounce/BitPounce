#include "bp_pch.h"
#include "ProjectSerializer.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <BitPounce/Core/FileSystem.h>

namespace BitPounce
{
	ProjectSerializer::ProjectSerializer(Ref<Project> project)
		: m_Project(project)
	{
		
	}

	bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
	{
		const auto& config = m_Project->GetConfig();

		nlohmann::json out = nlohmann::json();
		
		{
			nlohmann::json project = nlohmann::json();
			project["Name"] = config.Name;
			project["StartScene"] = config.StartScene;
			project["AssetDirectory"] = std::string(config.AssetDirectory.string());
			project["AssetRegistryPath"] = std::string(config.AssetRegistryPath.string());
			if(!config.AssetPacks.empty())
			{
				auto assetPacks = project.array();
				for (auto&& path: config.AssetPacks)
				{
					assetPacks.push_back(path.string());
				}
				project["AssetPacks"] = assetPacks;
				
			}
			

			out["Project"] = project;
		}

		std::ofstream fout(filepath);
		fout << out.dump(1, '	');

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		auto& config = m_Project->GetConfig();
		BP_CORE_INFO("WHY");

		BufferBase buffer = FileSystem::LoadFile(filepath);
		BP_CORE_INFO("WHY");
		nlohmann::json data = nlohmann::json::parse(std::string(buffer.As<const char>(), buffer.Size));
		BP_CORE_INFO("WHY");

		auto&& projectNode = data["Project"];
		BP_CORE_INFO("WHY");
		config.Name = projectNode["Name"].get<std::string>();
		BP_CORE_INFO("WHY");
		config.StartScene = projectNode["StartScene"].get<std::string>();
		BP_CORE_INFO("WHY");
		config.AssetDirectory = projectNode["AssetDirectory"].get<std::string>();
		BP_CORE_INFO("WHY");
		config.AssetRegistryPath = projectNode["AssetRegistryPath"].get<std::string>();
		BP_CORE_INFO("WHY");
		if(projectNode.contains("AssetPacks"))
		{
			BP_CORE_INFO("WHY");
			auto&& assetPacks = projectNode["AssetPacks"];
			BP_CORE_INFO("WHY");
			for (auto&& path: assetPacks)
			{
				config.AssetPacks.push_back(path.get<std::string>());
			}
		}
		return true;
	}


}