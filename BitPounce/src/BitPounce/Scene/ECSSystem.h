#pragma once
#include "BitPounce/Core/System.h"
#include "SceneSerializerUtils.h"
#include <BitPounce/Renderer/EditorCamera.h>

namespace BitPounce
{
	class Entity;
    class Scene;

	class ECSSystem : public System
	{
	public:
		virtual void OnEditorPropImguiDraw(Entity& entity) {};

		// HACK because c++ is c++, and templates using you can not use virtual for this idk why and this is why c++ is evil 😈!!!!
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		virtual void AddComponentPopupImguiDraw(Entity& ent) {};
		virtual void Serialize(nlohmann::json& json) {};
		virtual void Deserialize(nlohmann::json& json) {};
		virtual void OnDrawEditor(Timestep& ts, EditorCamera& cam) {};
		virtual void OnRuntimeStart() {};
		virtual void OnRuntimeStop() {};
	protected:
		Scene* m_Scene;
	private:
	
		void INIT_INT(Scene* scene);
		friend class ECSSystemManager;
	};
}