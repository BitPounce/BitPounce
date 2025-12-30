#include "BitPounce/Core/Input.h"

namespace BitPounce
{
    class LinuxInput : public Input
    {
        public:
            virtual bool IsKeyPressedImpl(Key key) override;
        private:

    };
}