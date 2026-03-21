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

namespace BitPounce {

	class Entity;
	struct CameraComponent;
	struct TransformComponent;

	class Scene
	{
	public:
		Scene(const std::string& name = std::string("Scene"));
		~Scene();

		Entity CreateEntity(const std::string& name = std::string("Entity"));
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnUpdateRuntime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);
		void OnEditorPropImguiDraw(Entity& entity);
		static Ref<Scene> Copy(Ref<Scene> other);
		void OnViewportResize(uint32_t width, uint32_t height);
		void AddComponentPopupImguiDraw(Entity& ent);

		template<typename SystemType, typename... Args>
        SystemType* AddSystem(Args&&... args)
        {
            
            return m_sysManager.AddSystem<SystemType>(std::forward<Args>(args)...);
        }

		void AddedAllSys();
		void Serialize(nlohmann::json& json);
		void Deserialize(nlohmann::json& json);
		ECSSystemManager& GetSysManager() { return m_sysManager; }
		void RemoveAll();

		std::string name;


		// USE ENITY, ONLY USE IN SYSTEMS
		entt::registry& GetRegistry(ECSSystem& sys) { (void)sys; return m_Registry; }

		std::pair<CameraComponent*, TransformComponent*> GetActiveCamera();
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

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		entt::registry m_Registry;
		ECSSystemManager m_sysManager;

		friend class Entity;
		friend class ECSSystem;
	};

}