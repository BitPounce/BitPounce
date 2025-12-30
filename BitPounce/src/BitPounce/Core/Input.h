#include "BitPounce/Core/KeyCode.h"


namespace BitPounce
{
	class Input
	{
		public:
			
			static bool IsKeyPressed(Key key) { return s_Instance->IsKeyPressedImpl(key); };
		
		protected:
			virtual bool IsKeyPressedImpl(Key key) = 0;
		private:
			static Input* s_Instance;
	};
}