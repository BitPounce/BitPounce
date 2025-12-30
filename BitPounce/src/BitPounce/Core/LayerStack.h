#pragma once

#include "BitPounce/Core/Base.h"
#include "Layer.h"

#include <vector>
#include <cstddef>

namespace BitPounce
{

	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end()   { return m_Layers.end(); }

	private:
		std::vector<Layer*> m_Layers;
		std::size_t m_LayerInsertIndex = 0;
	};

}
