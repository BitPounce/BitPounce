#include <bp_pch.h>
#include "UIRoot.h"
#include "BitPounce/Renderer/Renderer2D.h"

namespace BitPounce
{
	UIRoot::UIRoot()
		: m_Position(0.0f), m_Scale(1.0f), m_Size(0.0f)
	{
	}

	UIRoot::~UIRoot() {}

	void UIRoot::Render(const glm::mat4& parentTransform)
	{
		glm::mat4 transform = glm::translate(parentTransform, glm::vec3(m_Position, 0.0f));
		transform = glm::scale(transform, glm::vec3(m_Scale, 1.0f));

		for (auto& element : m_Elements)
		{
			if (element && element->IsVisible())
				element->Render(transform);
		}
	}

	void UIRoot::Update(Timestep ts)
	{
		for (auto& element : m_Elements)
			element->Update(ts);
	}

	bool UIRoot::OnEvent(Event& event, const glm::mat4& parentTransform)
	{
		glm::mat4 transform = glm::translate(parentTransform, glm::vec3(m_Position, 0.0f));
		transform = glm::scale(transform, glm::vec3(m_Scale, 1.0f));

		// Pass event to children (in reverse order so topmost gets first chance)
		for (auto it = m_Elements.rbegin(); it != m_Elements.rend(); ++it)
		{
			if (*it && (*it)->OnEvent(event, transform))
				return true;
		}
		return false;
	}

	void UIRoot::AddElement(std::unique_ptr<UIElement> element)
	{
		m_Elements.push_back(std::move(element));
	}

	void UIRoot::RemoveElement(UIElement* element)
	{
		auto it = std::find_if(m_Elements.begin(), m_Elements.end(),
			[element](const std::unique_ptr<UIElement>& ptr) { return ptr.get() == element; });
		if (it != m_Elements.end())
			m_Elements.erase(it);
	}
}