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
				entityJson["parent"] = parent.GetUUID().operator uint64_t();
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
		nlohmann::json json = SceneToJson();

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

	static void SerializeRuntimeEntity(BitPouncePack::PackScene& packScene, Entity entity)
	{
		BitPouncePack::SerializationObject entityObj = BitPouncePack::SerializationObject();
		entityObj.Set("UUID", BitPouncePack::SerializationType::Uint64, entity.GetUUID().operator uint64_t());
		entityObj.Set("entityId", BitPouncePack::SerializationType::Uint, entity.operator unsigned int());

		BitPouncePack::SerializationObjectArray components = BitPouncePack::SerializationObjectArray();

		if(entity.HasComponent<TransformComponent>())
		{
			TransformComponent comp = entity.GetComponent<TransformComponent>();
			BitPouncePack::SerializationObject transformComponent = BitPouncePack::SerializationObject();
			transformComponent.Set<std::string>("Type", BitPouncePack::SerializationType::String, "TransformComponent");
			transformComponent.Set("Translation", BitPouncePack::SerializationType::Vector3, comp.Translation);
			transformComponent.Set("Scale", BitPouncePack::SerializationType::Vector3, comp.Scale);
			if(entity.GetParent())
			{
				auto&& parent = entity.GetParent();
				entityObj.Set("parentUUID", BitPouncePack::SerializationType::Uint64,
				  parent.GetUUID().operator uint64_t());
			}
			transformComponent.Set("Rotation", BitPouncePack::SerializationType::Vector3, comp.Rotation);
			components.Add(transformComponent);
		}

		

		if(entity.HasComponent<TagComponent>())
		{
			TagComponent comp = entity.GetComponent<TagComponent>();
			BitPouncePack::SerializationObject tagComponent = BitPouncePack::SerializationObject();
			tagComponent.Set<std::string>("Type", BitPouncePack::SerializationType::String, "TagComponent");
			tagComponent.Set("Tag", BitPouncePack::SerializationType::String,comp.Tag);
			components.Add(tagComponent);
		}
		entityObj.Set("Components", BitPouncePack::SerializationType::SerializationObjectArray, components);
		packScene.Entities.Add(entityObj);
	}

	BitPouncePack::PackScene SceneSerializer::SerializeRuntime()
	{
		BitPouncePack::PackScene packScene = BitPouncePack::PackScene();
		packScene.Name = m_Scene->name;
		packScene.AssetHandle = m_Scene->Handle;
		packScene.Entities = BitPouncePack::SerializationObjectArray();
		packScene.Info = BitPouncePack::SerializationObject();

		nlohmann::json json = SceneToJson();

		std::string output = json.dump(1, '\t'); // JSON pretty print
		packScene.Info.Set<std::string>("FileData", BitPouncePack::SerializationType::String, output);

		return packScene;
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
		

		return Deserialize_IN(json);
	}

	bool SceneSerializer::DeserializeRuntime(const BitPouncePack::PackScene& packScene)
	{
		std::string fileData = packScene.Info.Get<std::string>("FileData", BitPouncePack::SerializationType::String);
		//BP_CORE_INFO("Loading Scene Data {0}, Name: {1}", fileData, packScene.Name);
		nlohmann::json json = nlohmann::json::parse(fileData);
		return Deserialize_IN(json);


		/*
		struct EntityLoadData
		{
			uint64_t uuid;
			std::string name;
			TransformComponent transform;
			bool hasTransform;
			uint64_t parentUUID;
		};

		std::vector<EntityLoadData> loadData;
		loadData.reserve(packScene.Entities.GetAll().size());

		for (const auto& entityObj : packScene.Entities.GetAll())
		{
			EntityLoadData data;
			data.uuid = entityObj.Get<uint64_t>("UUID", BitPouncePack::SerializationType::Uint64);
			data.name = "Entity";
			data.hasTransform = false;
			data.parentUUID = 0;

			if (entityObj.Contains("Components"))
			{
				auto components = entityObj.Get<BitPouncePack::SerializationObjectArray>(
					"Components", BitPouncePack::SerializationType::SerializationObjectArray);
				for (const auto& comp : components.GetAll())
				{
					std::string type = comp.Get<std::string>("Type", BitPouncePack::SerializationType::String);
					if (type == "TagComponent")
					{
						data.name = comp.Get<std::string>("Tag", BitPouncePack::SerializationType::String);
					}
					else if (type == "TransformComponent")
					{
						data.hasTransform = true;
						// Keep your original Vector3 reads – they work
						if (comp.Contains("Translation"))
							data.transform.Translation = comp.Get<glm::vec3>("Translation", BitPouncePack::SerializationType::Vector3);
						if (comp.Contains("Scale"))
							data.transform.Scale = comp.Get<glm::vec3>("Scale", BitPouncePack::SerializationType::Vector3);
						if (comp.Contains("Rotation"))
							data.transform.Rotation = comp.Get<glm::vec3>("Rotation", BitPouncePack::SerializationType::Vector3);
					}
				}
			}

			if (entityObj.Contains("parentUUID"))
				data.parentUUID = entityObj.Get<uint64_t>("parentUUID", BitPouncePack::SerializationType::Uint64);

			loadData.push_back(data);
		}

		std::unordered_map<uint64_t, Entity> uuidToEntity;
		for (auto& data : loadData)
		{
			Entity entity = m_Scene->CreateEntityWithUUID(UUID(data.uuid), data.name);
			uuidToEntity[data.uuid] = entity;
		}

		for (auto& data : loadData)
		{
			if (data.parentUUID != 0)
			{
				auto childIt = uuidToEntity.find(data.uuid);
				auto parentIt = uuidToEntity.find(data.parentUUID);
				if (childIt != uuidToEntity.end() && parentIt != uuidToEntity.end())
				{
					childIt->second.SetParent(parentIt->second);
				}
				else
				{
					BP_CORE_WARN("Parent link failed: child UUID %llx or parent UUID %llx not found",
								 data.uuid, data.parentUUID);
				}
			}
		}

		for (auto& data : loadData)
		{
			if (data.hasTransform)
			{
				auto it = uuidToEntity.find(data.uuid);
				if (it != uuidToEntity.end())
				{
					it->second.GetComponent<TransformComponent>() = data.transform;
				}
			}
		}

		m_Scene->DeserializeRuntime((BitPouncePack::PackScene*)&packScene);

		return true;*/
		
	}
	nlohmann::json SceneSerializer::SceneToJson()
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

		return json;
	}
	bool SceneSerializer::Deserialize_IN(nlohmann::json json)
	{
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
} // namespace BitPounce