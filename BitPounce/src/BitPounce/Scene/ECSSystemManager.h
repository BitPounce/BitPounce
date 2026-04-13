#pragma once
#include "BitPounce/Core/SystemManager.h"
#include "ECSSystem.h"
#include "SceneSerializerUtils.h"
#include <BitPounce/Renderer/EditorCamera.h>
#include <entt/entt.hpp>

namespace BitPounce
{
	class Scene;

	class ECSSystemManager : public SystemManager
	{
	public:
		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnEditorPropImguiDraw(Entity& entity);
		void AddComponentPopupImguiDraw(Entity& ent);
		void OnRemoveEntity(Entity& ent);
		void Serialize(nlohmann::json& json);
		void Deserialize(nlohmann::json& json);
		void OnDrawEditor(Timestep& ts, EditorCamera& cam);
		void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap);
	protected:
		Scene* m_Scene;

        virtual void AddSys_in(System* sys) override;
	private:
        friend class Scene;
		friend class SceneImporter;
	};
}