#pragma once

#include "Scene.h"
#include <BitPounce/Core/Logger.h>
#include <entt/entt.hpp>

namespace BitPounce
{
	struct TransformComponent;
	struct ChildrenComponent;
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

			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);

			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			BP_CORE_ASSERT(HasComponent<T>(), "Entity does not has component");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		const T& GetComponent() const
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
		bool HasComponent() const
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

		operator bool() const { return m_EntityHandle != entt::null || m_Scene; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

		// Transform and Children Components
		TransformComponent& GetTransform();
		std::vector<Entity>& GetChildren();
		Entity& GetParent();
		std::string& GetName();
		void SetParent(Entity& ent);
		UUID GetUUID();

		void Destroy() { m_Scene->DestroyEntity(*this); }

	private:
		void OnDestroy();

		entt::entity m_EntityHandle;
		Scene* m_Scene;

		friend class Scene;
	};
}