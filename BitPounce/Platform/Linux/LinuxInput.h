#include "BitPounce/Core/Input.h"

namespace BitPounce
{
    class LinuxInput : public Input
    {
        protected:
            virtual bool IsKeyPressedImpl(Key key) override;

            virtual bool IsMouseButtonPressedImpl(int button) override;
		    virtual glm::vec2 GetMousePositionImpl() override;
		    virtual float GetMouseXImpl() override;
		    virtual float GetMouseYImpl() override;
        private:

    };
}