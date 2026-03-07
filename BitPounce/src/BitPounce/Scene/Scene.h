#pragma once

#include <entt/entt.hpp>

#include "ECSSystemManager.h"
#include "BitPounce/Core/Timestep.h"
#include <utility>
#include <optional>

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

		template<typename SystemType>
        SystemType* AddSystem()
        {
            
            return m_sysManager.AddSystem<SystemType>();
        }

		void AddedAllSys();


		// USE ENITY, ONLY USE IN SYSTEMS
		entt::registry& GetRegistry(ECSSystem& sys) { (void)sys; return m_Registry; }

		std::pair<CameraComponent*, TransformComponent*> GetActiveCamera();
	private:
		entt::registry m_Registry;
		ECSSystemManager m_sysManager;

		friend class Entity;
	};

}