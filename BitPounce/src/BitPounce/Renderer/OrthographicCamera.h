#pragma once

#include <glm/glm.hpp>

namespace BitPounce {

	class OrthographicCamera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		const glm::vec3& GetPosition() const { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; isDirty = true; }

		float GetRotation() const { return m_Rotation; }
		void SetRotation(float rotation) { m_Rotation = rotation; isDirty = true; }

		const glm::mat4& GetProjectionMatrix() const { CheckDirty(); return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { CheckDirty(); return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { CheckDirty(); return m_ViewProjectionMatrix; }
	private:
		void RecalculateViewMatrix() const;
		void CheckDirty() const;
	private:
		mutable glm::mat4 m_ProjectionMatrix;
		mutable glm::mat4 m_ViewMatrix;
		mutable glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		float m_Rotation = 0.0f;
		mutable bool isDirty = true;
	};

}