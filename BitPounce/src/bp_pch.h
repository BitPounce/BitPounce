#pragma once
#include "BitPounce/Core/PlatformDetection.hpp"
#include "BitPounce/Core/Logger.h"
#include "BitPounce/Events/Event.h"
#include "BitPounce/Events/ApplicationEvent.h"
#include "BitPounce/Events/KeyEvent.h"
#include "BitPounce/Events/MouseEvent.h"

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#ifdef BP_PLATFORM_WINDOWS
	#include <Windows.h>
#elifdef BP_PLATFORM_WEB 
	#include <emscripten.h>
#endif 
