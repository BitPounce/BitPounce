#pragma once
#include "BitPounce/Core/SystemManager.h"
#include "ECSSystem.h"
#include "SceneSerializerUtils.h"
#include "BitPouncePack/BitPouncePack.h"
#include <BitPounce/Renderer/EditorCamera.h>
#include <entt/entt.hpp>

namespace BitPounce
{
	class Scene;

	class ECSSystemManager : public SystemManager
	{
	public:
		virtual void OnRuntimeStart();
		virtual void OnRuntimeStop();
		virtual void SerializeRuntime(BitPouncePack::PackScene* packScene);
		virtual void OnEditorPropImguiDraw(Entity& entity);
		//virtual void OnImguiDraw(Timestep ts);
		virtual void AddComponentPopupImguiDraw(Entity& ent);
		virtual void OnRemoveEntity(Entity& ent);
		virtual void Serialize(nlohmann::json& json);
		virtual void Deserialize(nlohmann::json& json);
		virtual void DeserializeRuntime(BitPouncePack::PackScene* packScene);
		virtual void OnDrawEditor(Timestep& ts, EditorCamera& cam);
		virtual void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap);
	protected:
		Scene* m_Scene;

        virtual void AddSys_in(System* sys) override;
	private:
        friend class Scene;
		friend class SceneImporter;
	};
}