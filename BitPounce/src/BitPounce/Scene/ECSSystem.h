#pragma once
#include "BitPounce/Core/System.h"

namespace BitPounce
{
	class Scene;

	class ECSSystem : public System
	{
	public:
		
	protected:
		Scene* m_Scene;
	private:
	
		void INIT_INT(Scene* scene);
		friend class ECSSystemManager;
	};
}