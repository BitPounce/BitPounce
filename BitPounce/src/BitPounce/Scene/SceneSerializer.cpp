#include <bp_pch.h>
#include "SceneSerializer.h"
#include "Entity.h"
#include "Scene.h"
#include "Components.h"

#define BAD_HACK_FOR_RANDOWM_32BIT_UNSIGNED_INTEGERS() (((uint32_t)(((uint16_t)rand())<<1 + ((uint16_t)rand()>>15)))<<16) + ((uint16_t)rand())<<1 + ((uint16_t)rand()>>15)

namespace BitPounce 
{
	static void SerializeEntity(nlohmann::json& json, Entity entity)
	{
		nlohmann::json entityJson;

		entityJson["ID"] = entity.GetUUID().operator uint64_t();
		entityJson["entityID"] = entity.operator unsigned int();

		if(entity.HasComponent<TransformComponent>())
		{
			TransformComponent comp = entity.GetComponent<TransformComponent>();
			nlohmann::json transformComponent = nlohmann::json();
			transformComponent["Translation"] = comp.Translation;
			transformComponent["Scale"] = comp.Scale;
			if(entity.GetParent())
			{
				auto&& parent = entity.GetParent();
				entityJson["parent"] = parent.operator unsigned int();
			}
			transformComponent["Rotation"] = comp.Rotation;
			entityJson["TransformComponent"] = transformComponent;
		}

		

		if(entity.HasComponent<TagComponent>())
		{
			TagComponent comp = entity.GetComponent<TagComponent>();
			nlohmann::json tagComponent = nlohmann::json();
			tagComponent["Tag"] = comp.Tag;
			entityJson["TagComponent"] = tagComponent;
		}
		json["Entities"].emplace_back(entityJson);
	}

	SceneSerializer::SceneSerializer(const Ref<Scene> &scene) 
	{
		m_Scene = scene;
	}
	void SceneSerializer::Serialize(const std::string &filepath) 
	{
		nlohmann::json json = nlohmann::json();
		json["Scene"] = "Untitled";
		json["Entities"] = json.array();

		;
		auto& registry = m_Scene->GetRegistry(*(ECSSystem*)(0));

		// iterate via view of all entities
		auto& storage = registry.storage<entt::entity>();
		for (auto entityID : storage) {
			if (!registry.valid(entityID))
			{
				continue;
			}
				
		
			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
			{
				continue;
			}
				
		
			SerializeEntity(json, entity);
		}

		m_Scene->Serialize(json);

		std::filesystem::create_directories(std::filesystem::path(filepath).parent_path());
		FILE* file = fopen(filepath.c_str(), "w");
    	if (!file)
    	{
			BP_CORE_ASSERT(0,"Failed to open file: {}", filepath.c_str());
    	    return;
    	}

    	std::string output = json.dump(1, '\t'); // JSON pretty print
    	size_t written = fwrite(output.c_str(), 1, output.size(), file);

    	fclose(file);
    	printf("%s\n", output.c_str()); // optional debug print
	}
	
	bool SceneSerializer::Deserialize(const std::string &filepath) 
    {
        if (!std::filesystem::exists(filepath))
        {
            BP_CORE_WARN("Scene file does not exist: {}", filepath);
            return false;
        }

        BufferBase buffer = FileSystem::LoadFile(filepath);

		std::string jsonStr((char*)buffer.Data, buffer.Size);
        nlohmann::json json = nlohmann::json::parse(jsonStr);
        if (!json.contains("Entities"))
            return false;

        auto& registry = m_Scene->GetRegistry(*(ECSSystem*)(0));

        for (auto& entityJson : json["Entities"])
        {
			std::string name = std::string("Entity");
			if(entityJson.contains("TagComponent"))
			{
				auto& tagJson = entityJson["TagComponent"];
				name = tagJson["Tag"];
			}
			uint64_t uuid = entityJson["ID"].get<uint64_t>();
            Entity entity = m_Scene->CreateEntityWithUUID(uuid, name);
			entityJson["entityOldID"] = entityJson["entityID"].get<unsigned int>();
			entityJson["entityID"] = entity.operator unsigned int();

            // TransformComponent
            if (entityJson.contains("TransformComponent"))
            {
                auto& transformJson = entityJson["TransformComponent"];
                TransformComponent& comp = entity.GetComponent<TransformComponent>();
                if (transformJson.contains("Translation"))
                    comp.Translation = transformJson["Translation"].get<glm::vec3>();
                if (transformJson.contains("Scale"))
                    comp.Scale = transformJson["Scale"].get<glm::vec3>();
                if (transformJson.contains("Rotation"))
                    comp.Rotation = transformJson["Rotation"].get<glm::vec3>();
            }
			
			
        }

		for (auto& entityJson : json["Entities"])
		{
			Entity entity = Entity{(entt::entity)entityJson["entityID"].get<unsigned int>(), m_Scene.get()};
			

			if (entityJson.contains("TransformComponent"))
			{
				auto& transformJson = entityJson["TransformComponent"];
                TransformComponent& comp = entity.GetComponent<TransformComponent>();

				if(entityJson.contains("parent"))
				{
					for (auto& entityJson2 : json["Entities"])
					{
						if(entityJson["parent"].get<unsigned int>() == entityJson2["entityOldID"].get<unsigned int>())
						{
							Entity e = Entity((entt::entity)entityJson2["entityID"].get<unsigned int>(), m_Scene.get());
							entity.SetParent(e);
						}
					}
				}
			}

			
		}

		m_Scene->Deserialize(json);

        return true;
    }
	bool SceneSerializer::DeserializeRuntime(const std::string &filepath) 
	{
		return false;
	}
} // namespace BitPounce