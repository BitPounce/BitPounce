#include "System.h"
#include <memory>

namespace BitPounce
{
    class SystemManager
    {

    public:

        template<typename SystemType>
        static SystemType* AddSystem()
        {
            SystemType* sys = new SystemType();
            AddSys_in(sys);
            return sys;
        }

        static void OnUpdate(Timestep& ts);
        static void OnEvent(Event& event);
        static void OnImGuiDraw();
        static void Start();
        static void Stop();

        static void StopSystem(System* sys);
        static void StartSystem(System* sys);

        static std::vector<System*> Get()
        {
            return s_systems;
        }
    private:
        static void AddSys_in(System* sys);

        static std::vector<System*> s_systems;
    };
    
    
}