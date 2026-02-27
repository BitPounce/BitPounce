#include <bp_pch.h>
#include "SystemManager.h"

namespace BitPounce
{
	std::vector<System*> SystemManager::s_systems = std::vector<System*>();

	void SystemManager::OnUpdate(Timestep& ts)
	{
		for (auto& sys : s_systems)
		{
			sys->OnUpdate(ts);
		}
	}

	void SystemManager::OnEvent(Event& event)
	{
		for (auto& sys : s_systems)
		{
			sys->OnEvent(event);
		}
	}

	void SystemManager::OnImGuiDraw()
	{
		for (auto& sys : s_systems)
		{
			sys->OnImGuiDraw();
		}
	}

    void SystemManager::Start()
    {
		for (auto& sys : s_systems)
		{
			sys->Start();
		}
    }

    void SystemManager::Stop()
    {
		for (auto& sys : s_systems)
		{
			sys->Stop();
		}
    }

    void SystemManager::StopSystem(System* sys)
	{
		for (auto* s : s_systems)
		{
			if (s == sys)
			{
				s->Stop();
				return;
			}
		}
	}

    void SystemManager::StartSystem(System* sys)
    {
		for (auto* s : s_systems)
		{
			if (s == sys)
			{
				s->Start();
				return;
			}
		}
    }

    void SystemManager::AddSys_in(System* sys)
	{
		s_systems.push_back(sys);
	}

}