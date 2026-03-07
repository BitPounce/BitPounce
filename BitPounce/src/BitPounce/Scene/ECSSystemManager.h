#pragma once
#include "BitPounce/Core/SystemManager.h"
#include "ECSSystem.h"

namespace BitPounce
{
	class Scene;

	class ECSSystemManager : public SystemManager
	{
	public:
		
	protected:
		Scene* m_Scene;

        virtual void AddSys_in(System* sys) override;
	private:
        friend class Scene;
	};
}