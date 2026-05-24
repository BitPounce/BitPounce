#pragma once

#include <vector>
#include <BitPounce/Core/Layer.h>
#include "UIRoot.h"
#include <BitPounce/Renderer/OrthographicCamera.h>

namespace BitPounce
{
	class UILayer : public Layer
	{
	public:
		UILayer();
		~UILayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep& ts) override;
		virtual void OnEvent(Event& event) override;

		void AddRoot(Ref<UIRoot> root);
		void RemoveRoot(Ref<UIRoot> root);

		// Convert screen coordinates to world UI coordinates (for mouse events)
		glm::vec2 ScreenToUIPosition(const glm::vec2& screenPos) const;

	private:
		std::vector<Ref<UIRoot>> m_Roots;
		OrthographicCamera m_Camera;
		bool m_CameraInitialized = false;
	};
}