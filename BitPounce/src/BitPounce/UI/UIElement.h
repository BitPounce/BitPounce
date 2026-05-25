#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <memory>
#include <functional>
#include "BitPounce/Renderer/Texture.h"
#include "BitPounce/Renderer/Font.h"
#include <BitPounce/Core/Bounds.h>
#include <BitPounce/Events/Event.h>
#include <BitPounce/Core/Timestep.h>
#include <BitPounce/Core/UUID.h>
#include <BitPounce/Core/FunctionArray.h>

namespace BitPounce
{
	using UIElementID = UUID;

	class UIElement
	{
	public:
		UIElement() = default;
		virtual ~UIElement() = default;

		virtual void Render(const glm::mat4& transform) = 0;
		virtual void Update(Timestep ts) {}
		virtual bool OnEvent(Event& event, const glm::mat4& transform) { return false; }

		void SetBounds(const FBounds2& bounds) { m_Bounds = bounds; }
		const FBounds2& GetBounds() const { return m_Bounds; }

		void SetVisible(bool visible) { m_Visible = visible; }
		bool IsVisible() const { return m_Visible; }

		void SetUserData(void* data) { m_UserData = data; }
		void* GetUserData() const { return m_UserData; }

		FBounds2 GetTransformedBounds(const glm::mat4& transform) const;

		UIElementID& GetID() { return m_Id; }
		void SetID(const UIElementID& id) { m_Id = id; }

	protected:
		FBounds2 m_Bounds;
		bool m_Visible = true;
		void* m_UserData = nullptr;
		UIElementID m_Id;
	};

	class UIRectElement : public UIElement
	{
	public:
		UIRectElement(const glm::vec4& color);
		virtual void Render(const glm::mat4& transform) override;
		void SetColor(const glm::vec4& color) { m_Color = color; }

	private:
		glm::vec4 m_Color;
	};

	class UIImageElement : public UIElement
	{
	public:
		UIImageElement(Ref<Texture2D> texture);
		virtual void Render(const glm::mat4& transform) override;
		void SetTexture(Ref<Texture2D> texture) { m_Texture = texture; }
		void SetTilingFactor(float tiling) { m_TilingFactor = tiling; }
		void SetTint(const glm::vec4& tint) { m_Tint = tint; }

	private:
		Ref<Texture2D> m_Texture;
		float m_TilingFactor = 1.0f;
		glm::vec4 m_Tint = glm::vec4(1.0f);
	};

	class UITextElement : public UIElement
	{
	public:
		UITextElement(const std::string& text, Ref<Font> font);
		virtual void Render(const glm::mat4& transform) override;
		void SetText(const std::string& text) { m_Text = text; }
		void SetFont(Ref<Font> font) { m_Font = font; }
		void SetColor(const glm::vec4& color) { m_Color = color; }
		void SetKerning(float kerning) { m_Kerning = kerning; }
		void SetLineSpacing(float spacing) { m_LineSpacing = spacing; }

	private:
		std::string m_Text;
		Ref<Font> m_Font;
		glm::vec4 m_Color = glm::vec4(1.0f);
		float m_Kerning = 0.0f;
		float m_LineSpacing = 0.0f;
	};

	class UIButtonElement : public UIElement
	{
	public:
		UIButtonElement(const FBounds2& bounds, std::unique_ptr<UIElement> content);
		virtual void Render(const glm::mat4& transform) override;
		virtual bool OnEvent(Event& event, const glm::mat4& transform) override;

		void SetOnClickCallback(FunctionArray<void()> callback) { m_OnClick = callback; }
		FunctionArray<void()>& GetOnClickCallback() { return m_OnClick; }

		void SetOnHoveredCallback(FunctionArray<void()> callback) { m_OnHovered = callback; }
		FunctionArray<void()>& GetOnHoveredCallback() { return m_OnHovered; }

		void SetOnNotHoveredCallback(FunctionArray<void()> callback) { m_OnNotHovered = callback; }
		FunctionArray<void()>& GetOnNotHoveredCallback() { return m_OnNotHovered; }
	private:
		std::unique_ptr<UIElement> m_Content;
		FunctionArray<void()> m_OnClick;
		FunctionArray<void()> m_OnHovered;
		FunctionArray<void()> m_OnNotHovered;
		bool m_IsHovered = false;
		bool m_IsPressed = false;
	};
}