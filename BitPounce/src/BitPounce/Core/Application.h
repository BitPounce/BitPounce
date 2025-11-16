#pragma once

#include "Base.h"

namespace BitPounce
{
	class Application
	{
	public:
		Application();
		virtual ~Application();
		
		int Run();
	private:
		void Update();
		static Application* s_Instance;
	};

	
}
extern BitPounce::Application* CreateApp();