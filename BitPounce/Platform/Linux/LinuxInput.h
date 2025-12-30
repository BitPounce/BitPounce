#include "BitPounce/Core/Input.h"

namespace BitPounce
{
    class LinuxInput : public Input
    {
        public:
            virtual bool IsKeyPressedImpl(KeyCode key) override;
        private:

    };
}