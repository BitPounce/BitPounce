#pragma once
#include "System.h"
#include <memory>

namespace BitPounce
{
    class SystemManager
    {

    public:
        SystemManager() {};
        ~SystemManager();

        template<typename SystemType, typename... Args>
        SystemType* AddSystem(Args&&... args)
        {
            
            SystemType* sys = new SystemType(std::forward<Args>(args)...);
            AddSys_in(sys);
            return sys;
        }

        virtual void OnUpdate(Timestep& ts);
        virtual void OnDraw(Timestep& ts);
        virtual void OnEvent(Event& event);
        virtual void OnImGuiDraw();
        virtual void Start();
        virtual void Stop();

        virtual void StopSystem(System* sys);
        virtual void StartSystem(System* sys);

        virtual std::vector<System*> Get()
        {
            return m_systems;
        }
    protected:
        virtual void AddSys_in(System* sys);

        std::vector<System*> m_systems;
    };
    
    
}