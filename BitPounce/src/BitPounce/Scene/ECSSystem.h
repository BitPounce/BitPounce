#pragma once
#include "BitPounce/Core/System.h"
#include "SceneSerializerUtils.h"
#include <entt/entt.hpp>
#include <BitPounce/Renderer/EditorCamera.h>
#include <BitPounce/Core/Logger.h>

namespace BitPounce
{
	class Entity;
    class Scene;

	struct IDComponent;

	class ECSSystem : public System
	{
	public:
		ECSSystem() {};

		virtual void OnEditorPropImguiDraw(Entity& entity) {};

		// HACK because c++ is c++, and templates using you can not use virtual for this idk why and this is why c++ is evil 😈!!!!
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		template<typename T>
		void OnComponentRemoved(Entity entity, T& component);

		virtual void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap) {};

		virtual void AddComponentPopupImguiDraw(Entity& ent) {};
		virtual void Serialize(nlohmann::json& json) {};
		virtual void Deserialize(nlohmann::json& json) {};
		virtual void OnDrawEditor(Timestep& ts, EditorCamera& cam) {};
		virtual void OnRuntimeStart() {};
		virtual void OnRemoveEntity(Entity& ent) {}
		virtual void OnRuntimeStop() {};

	protected:
		Scene* m_Scene;

		template<typename Component>
		static void CopyComponentBASE(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
		{
			auto view = src.view<Component>();
			for (auto e : view)
			{
				UUID uuid = src.get<IDComponent>(e).ID;
				BP_CORE_ASSERT(enttMap.find(uuid) != enttMap.end(), "");
				entt::entity dstEnttID = enttMap.at(uuid);
			
				auto& component = src.get<Component>(e);
				dst.emplace_or_replace<Component>(dstEnttID, component);
			}
		}
	private:
	
		void INIT_INT(Scene* scene);
		friend class ECSSystemManager;
		friend class Scene;
	};
}