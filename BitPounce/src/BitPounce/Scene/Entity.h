#pragma once

#include "Scene.h"

#include <entt/entt.hpp>

namespace BitPounce
{
	class Entity
	{
	public:
		Entity(entt::entity handle, Scene* scene);
		Entity();
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			BP_CORE_ASSERT(!HasComponent<T>(), "Entity already has component");

			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			BP_CORE_ASSERT(HasComponent<T>(), "Entity does not has component");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			BP_CORE_ASSERT(m_EntityHandle != entt::null, "Entity is null");
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			BP_CORE_ASSERT(HasComponent<T>(), "Entity does not has component");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}
	
	private:
		entt::entity m_EntityHandle;
		Scene* m_Scene;
	};
}