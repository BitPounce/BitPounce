#include <BitPounce/Core/Base.h>
#include <BitPounce/Core/Timestep.h>
#include <BitPounce/Events/Event.h>

namespace BitPounce
{
    class System
    {
    public:

        virtual void Start() {};
        virtual void Stop() {};

        virtual void OnUpdate(Timestep& ts) {};
        virtual void OnDraw(Timestep& ts) {};
        virtual void OnEvent(Event& event) {};
        virtual void OnImGuiDraw() {};

        std::string GetName() { return m_name; }

    protected:
        std::string m_name;
    };
}