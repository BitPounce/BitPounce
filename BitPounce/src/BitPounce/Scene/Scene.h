#pragma once

#include <entt/entt.hpp>

#include "ECSSystemManager.h"
#include "BitPounce/Core/Timestep.h"
#include "BitPounce/Core/UUID.h"
#include <utility>
#include <optional>
#include <BitPounce/Events/Event.h>
#include <BitPounce/Events/ApplicationEvent.h>
#include "SceneSerializerUtils.h"
#include "BitPouncePack/BitPouncePack.h"
#include "BitPounce/Asset/Asset.h"

namespace BitPounce {

	class Entity;
	struct CameraComponent;
	struct TransformComponent;


	class Scene: public Asset
	{
	public:
		Scene(const std::string& name = std::string("Scene"));
		virtual ~Scene();

		virtual Entity CreateEntity(const std::string& name = std::string("Entity"));
		virtual Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		virtual void DestroyEntity(Entity entity);
		virtual Entity FindEntityByUUID(UUID uuid) const;

		virtual void OnRuntimeStart();
		virtual void OnRuntimeStop();
		virtual void OnUpdateRuntime(Timestep ts);
		virtual void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		virtual void OnUpdate(Timestep ts);
		virtual void OnImguiDraw(Timestep ts);
		virtual void OnEvent(Event& e);
		virtual void OnEditorPropImguiDraw(Entity& entity);
		static Ref<Scene> Copy(Ref<Scene> other);
		virtual void OnViewportResize(uint32_t width, uint32_t height);
		virtual std::string GetName() const { return name; }
		virtual void AddComponentPopupImguiDraw(Entity& ent);

		template<typename SystemType, typename... Args>
        SystemType* AddSystem(Args&&... args)
        {
            
            return m_sysManager.AddSystem<SystemType>(std::forward<Args>(args)...);
        }

		virtual void AddedAllSys();
		virtual void Serialize(nlohmann::json& json);
		virtual void SerializeRuntime(BitPouncePack::PackScene* packScene);
		virtual void Deserialize(nlohmann::json& json);
		virtual void DeserializeRuntime(BitPouncePack::PackScene* packScene);
		virtual ECSSystemManager& GetSysManager() { return m_sysManager; }
		virtual void RemoveAll();

		std::string name;


		// USE ENITY, ONLY USE IN SYSTEMS
		entt::registry& GetRegistry(ECSSystem& sys) { (void)sys; return m_Registry; }
		virtual AssetType GetType() const override
		{
			return AssetType::Scene;
		}

		virtual std::pair<CameraComponent*, TransformComponent*> GetActiveCamera();
	private:
		template<typename T>
		void OnComponentAdded(Entity& entity, T& component)
		{
			for(System* sys : m_sysManager.Get())
			{
				((ECSSystem*)sys)->OnComponentAdded<T>(entity, component);
			}
		}
	private:
	
		bool OnResize(WindowResizeEvent& e);

		// This has to be set by the user!
		uint32_t m_ViewportWidth = 1, m_ViewportHeight = 1;
		entt::registry m_Registry;
		ECSSystemManager m_sysManager;

		friend class Entity;
		friend class ECSSystem;
	};

}