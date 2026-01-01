#include "BitPounce/Core/Input.h"

namespace BitPounce
{
    class WebInput : public Input
    {
        public:
            virtual bool IsKeyPressedImpl(Key key) override;
        private:

    };
}