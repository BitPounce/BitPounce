#pragma once

#include <glm/glm.hpp>
#include <string>

namespace BitPounce
{
	class ImGuiUtils
	{
	public:
		static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	};
}
