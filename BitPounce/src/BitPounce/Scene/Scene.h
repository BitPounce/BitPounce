#pragma once

#include <entt/entt.hpp>

#include "ECSSystemManager.h"
#include "BitPounce/Core/Timestep.h"

namespace BitPounce {

	class Scene
	{
	public:
		Scene();
		~Scene();

		entt::entity CreateEntity();

		// TEMP
		entt::registry& Reg() { return m_Registry; }

		void OnUpdate(Timestep ts);

		template<typename SystemType>
        SystemType* AddSystem()
        {
            
            return m_sysManager.AddSystem<SystemType>();
        }

		void AddedAllSys();
	private:
		entt::registry m_Registry;
		ECSSystemManager m_sysManager;
	};

}