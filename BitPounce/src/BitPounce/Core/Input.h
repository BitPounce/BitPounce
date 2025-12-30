#include "BitPounce/Core/Base.h"

// TODO: make unicode!
typedef uint16_t KeyCode;

namespace BitPounce
{
	class Input
	{
		public:
			
			static bool IsKeyPressed(KeyCode key) { return s_Instance->IsKeyPressedImpl(key); };
		
		protected:
			virtual bool IsKeyPressedImpl(KeyCode key) = 0;
		private:
			static Input* s_Instance;
	};
}