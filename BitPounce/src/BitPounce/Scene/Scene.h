#pragma once

#include <entt/entt.hpp>

#include "ECSSystemManager.h"
#include "BitPounce/Core/Timestep.h"
#include <utility>
#include <optional>
#include <BitPounce/Events/Event.h>
#include <BitPounce/Events/ApplicationEvent.h>

namespace BitPounce {

	class Entity;
	struct CameraComponent;
struct TransformComponent;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string("Entity"));


		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);
		void OnEditorPropImguiDraw(Entity& entity);
		void OnViewportResize(uint32_t width, uint32_t height);

		template<typename SystemType, typename... Args>
        SystemType* AddSystem(Args&&... args)
        {
            
            return m_sysManager.AddSystem<SystemType>(std::forward<Args>(args)...);
        }

		void AddedAllSys();


		// USE ENITY, ONLY USE IN SYSTEMS
		entt::registry& GetRegistry(ECSSystem& sys) { (void)sys; return m_Registry; }

		std::pair<CameraComponent*, TransformComponent*> GetActiveCamera();
	private:
	
		bool OnResize(WindowResizeEvent& e);

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		entt::registry m_Registry;
		ECSSystemManager m_sysManager;

		friend class Entity;
	};

}