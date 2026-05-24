#include <bp_pch.h>
#include "UILayer.h"
#include "BitPounce/Core/Application.h"
#include "BitPounce/Events/ApplicationEvent.h"
#include "BitPounce/Renderer/Renderer2D.h"

namespace BitPounce
{
	UILayer::UILayer()
		: Layer("UI Layer"), m_Camera(0, 1, 0, 1)
	{
	}

	UILayer::~UILayer() {}

	void UILayer::OnAttach()
	{
	    auto& app = Application::Get();
	    auto& window = app.GetWindow();

	    float width = (float)window.GetWidth();
	    float height = (float)window.GetHeight();

		

	    m_Camera = OrthographicCamera(0.0f, width, height, 0.0f);

	    m_Camera.SetPosition({ 0.0f, 0.0f, 0.0f });
	    m_CameraInitialized = true;
	}

	void UILayer::OnDetach()
	{
		m_Roots.clear();
	}

	void UILayer::OnUpdate(Timestep& ts)
	{
		if (!m_CameraInitialized) return;

		static float Timer = 0.0f;
		Timer += ts;
		m_Camera.SetPosition({ 0.0f, 0.0f, 0.0f });
		Renderer2D::BeginScene(m_Camera);

		for (auto& root : m_Roots)
		{
			if (root)
			{
				root->Update(ts);
				root->Render();
			}
		}

		Renderer2D::EndScene();
	}

	void UILayer::OnEvent(Event& event)
	{
		for (auto it = m_Roots.rbegin(); it != m_Roots.rend(); ++it)
		{
			if (*it && (*it)->OnEvent(event))
			{
				event.Handled = true;
				break;
			}
		}

		if (event.GetEventType() == EventType::WindowResize)
		{
			WindowResizeEvent& resize = (WindowResizeEvent&)event;
			m_Camera.SetProjection(0.0f, (float)resize.GetWidth(), (float)resize.GetHeight(), 0.0f);
		}
	}

	void UILayer::AddRoot(Ref<UIRoot> root)
	{
		m_Roots.push_back(root);
	}

	void UILayer::RemoveRoot(Ref<UIRoot> root)
	{
		auto it = std::find(m_Roots.begin(), m_Roots.end(), root);
		if (it != m_Roots.end())
			m_Roots.erase(it);
	}

	glm::vec2 UILayer::ScreenToUIPosition(const glm::vec2& screenPos) const
	{
		return screenPos;
	}
}