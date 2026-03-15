#pragma once
#include "BitPounce/Core/KeyCode.h"
#include "BitPounce/Core/MouseCode.h"
#include <glm/glm.hpp>

namespace BitPounce
{
	class Input
	{
		public:
			
			static bool IsKeyPressed(Key key);

			static bool IsMouseButtonPressed(int button);
			static glm::vec2 GetMousePosition();
			static float GetMouseX();
			static float GetMouseY();

	};
}