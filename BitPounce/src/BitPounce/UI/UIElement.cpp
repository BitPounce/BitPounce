#include <bp_pch.h>
#include "UIElement.h"
#include "BitPounce/Renderer/Renderer2D.h"
#include "BitPounce/Core/Application.h"
#include "BitPounce/Events/MouseEvent.h"

namespace BitPounce
{
	// Helper
	FBounds2 UIElement::GetTransformedBounds(const glm::mat4& transform) const
	{
		glm::vec2 minWorld = transform * glm::vec4(m_Bounds.min, 0.0f, 1.0f);
		glm::vec2 maxWorld = transform * glm::vec4(m_Bounds.max, 0.0f, 1.0f);
		FBounds2 result;
		result.min = glm::min(minWorld, maxWorld);
		result.max = glm::max(minWorld, maxWorld);
		return result;
	}

	// UIRectElement
	UIRectElement::UIRectElement(const glm::vec4& color)
		: m_Color(color) {}

	void UIRectElement::Render(const glm::mat4& transform)
	{
		if (!m_Visible) return;
		glm::mat4 finalTransform = glm::translate(transform, glm::vec3(m_Bounds.center(), 0.0f));
		finalTransform = glm::scale(finalTransform, glm::vec3(m_Bounds.size(), 1.0f));
		Renderer2D::DrawQuad(finalTransform, m_Color);
	}

	// UIImageElement
	UIImageElement::UIImageElement(Ref<Texture2D> texture)
		: m_Texture(texture) {}

	void UIImageElement::Render(const glm::mat4& transform)
	{
		if (!m_Visible || !m_Texture) return;
		glm::mat4 finalTransform = glm::translate(transform, glm::vec3(m_Bounds.center(), 0.0f));
		finalTransform = glm::scale(finalTransform, glm::vec3(m_Bounds.size(), 1.0f));
		Renderer2D::DrawQuad(finalTransform, m_Texture, m_Tint, m_TilingFactor);
	}

	// UITextElement
	UITextElement::UITextElement(const std::string& text, Ref<Font> font)
		: m_Text(text), m_Font(font) {}

	void UITextElement::Render(const glm::mat4& transform)
	{
		if (!m_Visible || !m_Font) return;

		glm::mat4 textTransform = glm::translate(transform, glm::vec3(m_Bounds.min, 0.0f));
		Renderer2D::TextParams params;
		params.Colour = m_Color;
		params.Kerning = m_Kerning;
		params.LineSpacing = m_LineSpacing;
		Renderer2D::DrawString(m_Text, m_Font, textTransform, params);
	}

	// UIButtonElement
	UIButtonElement::UIButtonElement(const FBounds2& bounds, std::unique_ptr<UIElement> content)
	{
		m_Bounds = bounds;
		m_Content = std::move(content);

		if (m_Content)
			m_Content->SetBounds(FBounds2(glm::vec2(0.0f), bounds.size()));
	}

	void UIButtonElement::Render(const glm::mat4& transform)
	{
		if (!m_Visible) return;
		glm::mat4 buttonTransform = glm::translate(transform, glm::vec3(m_Bounds.min, 0.0f));
		if (m_Content)
			m_Content->Render(buttonTransform);
	}

	bool UIButtonElement::OnEvent(Event& event, const glm::mat4& transform)
	{
		if (!m_Visible) return false;

		FBounds2 worldBounds = GetTransformedBounds(transform);
		glm::vec2 mousePos = Input::GetMousePosition();

		if (event.GetEventType() == EventType::MouseMoved)
		{
			bool hovered = worldBounds.contains(mousePos);
			if (hovered != m_IsHovered)
			{
				m_IsHovered = hovered;

				if(m_OnHovered) { m_OnHovered(); }
			}
			else
			{
				if(m_OnNotHovered) { m_OnNotHovered(); }
			}
			
		}
		else if (event.GetEventType() == EventType::MouseButtonPressed)
		{
			MouseButtonPressedEvent& btnEvent = (MouseButtonPressedEvent&)event;
			if (btnEvent.GetMouseButton() == 0 && worldBounds.contains(mousePos))
			{
				m_IsPressed = true;
				return true;
			}
		}
		else if (event.GetEventType() == EventType::MouseButtonReleased)
		{
			MouseButtonReleasedEvent& btnEvent = (MouseButtonReleasedEvent&)event;
			if (btnEvent.GetMouseButton() == 0 && m_IsPressed)
			{
				m_IsPressed = false;
				if (worldBounds.contains(mousePos) && m_OnClick)
					m_OnClick();
				return true;
			}
			m_IsPressed = false;
		}
		return false;
	}
}