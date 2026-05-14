#pragma once

#include <glm/glm.hpp>

namespace BitPounce 
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			: m_Projection(projection) {}

		const glm::mat4& GetProjection() const { return m_Projection; } ;

		virtual ~Camera() = default;

		static glm::vec2 PixelToWorld(glm::ivec2 pixel, const glm::mat4& projection, const glm::ivec2 res);

	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};
}