#include "BitPounce/Core/KeyCode.h"
#include <glm/glm.hpp>

namespace BitPounce
{
	class Input
	{
		public:
			
			static bool IsKeyPressed(Key key) { return s_Instance->IsKeyPressedImpl(key); };

			inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
			inline static glm::vec2 GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
			inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
			inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

			
		
		protected:
			virtual bool IsKeyPressedImpl(Key key) = 0;
			virtual bool IsMouseButtonPressedImpl(int button) = 0;
			virtual glm::vec2 GetMousePositionImpl() = 0;
			virtual float GetMouseXImpl() = 0;
			virtual float GetMouseYImpl() = 0;
		private:
			static Input* s_Instance;
	};
}