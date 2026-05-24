#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "UIElement.h"

namespace BitPounce
{
	class UIRoot
	{
	public:
		UIRoot();
		~UIRoot();

		void Render(const glm::mat4& parentTransform = glm::mat4(1.0f));
		void Update(Timestep ts);

		// Event handling - returns true if event was consumed
		bool OnEvent(Event& event, const glm::mat4& transform = glm::mat4(1.0f));

		void AddElement(std::unique_ptr<UIElement> element);
		void RemoveElement(UIElement* element);

		void SetPosition(const glm::vec2& position) { m_Position = position; }
		void SetScale(const glm::vec2& scale) { m_Scale = scale; }
		void SetSize(const glm::vec2& size) { m_Size = size; }

		const glm::vec2& GetPosition() const { return m_Position; }
		const glm::vec2& GetScale() const { return m_Scale; }
		const glm::vec2& GetSize() const { return m_Size; }

	private:
		glm::vec2 m_Position;
		glm::vec2 m_Scale;
		glm::vec2 m_Size;
		std::vector<std::unique_ptr<UIElement>> m_Elements;
	};
}