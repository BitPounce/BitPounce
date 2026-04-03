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

			out["Project"] = project;
		}

		std::ofstream fout(filepath);
		fout << out.dump(1, '	');

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		auto& config = m_Project->GetConfig();

		BufferBase buffer = FileSystem::LoadFile(filepath);
		nlohmann::json data = nlohmann::json::parse(std::string(buffer.As<const char>(), buffer.Size));

		auto&& projectNode = data["Project"];
		config.Name = projectNode["Name"].get<std::string>();
		config.StartScene = projectNode["StartScene"].get<std::string>();
		config.AssetDirectory = projectNode["AssetDirectory"].get<std::string>();
		return true;
	}


}