#pragma once
#include "BitPounce/Core/SystemManager.h"
#include "ECSSystem.h"
#include "SceneSerializerUtils.h"

namespace BitPounce
{
	class Scene;

	class ECSSystemManager : public SystemManager
	{
	public:
		void OnEditorPropImguiDraw(Entity& entity);
		void AddComponentPopupImguiDraw(Entity& ent);
		void Serialize(nlohmann::json& json);
		void Deserialize(nlohmann::json& json);
	protected:
		Scene* m_Scene;

        virtual void AddSys_in(System* sys) override;
	private:
        friend class Scene;
	};
}