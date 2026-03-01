#pragma once

#include "BitPounce/Core/Layer.h"

namespace BitPounce
{
	class ImGuiLayer : public Layer
	{
		public:
			ImGuiLayer();
			~ImGuiLayer();

			virtual void OnAttach() override;
			virtual void OnDetach() override;
			virtual void OnEvent(Event& e) override;
			virtual void OnImGuiRender() override;

			void BlockEvents(bool block) { m_BlockEvents = block; }

			void Begin();
			void End();

		private:
			bool m_BlockEvents = true;
	};
}