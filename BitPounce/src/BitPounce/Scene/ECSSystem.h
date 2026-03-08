#pragma once
#include "BitPounce/Core/System.h"

namespace BitPounce
{
	class Entity;
    class Scene;

	class ECSSystem : public System
	{
	public:
		virtual void OnEditorPropImguiDraw(Entity& entity) {};
	protected:
		Scene* m_Scene;
	private:
	
		void INIT_INT(Scene* scene);
		friend class ECSSystemManager;
	};
}