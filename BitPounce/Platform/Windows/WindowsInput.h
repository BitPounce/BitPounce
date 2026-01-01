#include "BitPounce/Core/Input.h"

namespace BitPounce
{
    class WindowsInput : public Input
    {
        public:
            virtual bool IsKeyPressedImpl(Key key) override;
        private:

    };
}