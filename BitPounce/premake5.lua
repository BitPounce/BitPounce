project "BitPounce"
	kind "StaticLib"
	language "C++"
	cppdialect "C++23"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "bp_pch.h"
	pchsource "src/bp_pch.cpp"

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
		
		"Platform/OpenGL/**.h",
		"Platform/OpenGL/**.hpp",
		"Platform/OpenGL/**.cpp",
		
		"../BitPounce/vendor/glad/src/gl.c"
	}

	includedirs
	{
		"src",
		"../BitPounce/vendor/spdlog/include",
		"../BitPounce/vendor/glad/include",
		"vendor/GLFW/include",
		"./"
	}
	
	links
	{
		"GLFW",
		
	}
	-- %{cfg.buildcfg}
	-- GLFW_INCLUDE_NONE
	defines {"GLFW_INCLUDE_NONE"}
	
	filter "files:vendor/**.*"
		flags { "NoPCH" }
	filter {}

	filter "system:windows"
		cppdialect "C++23"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"BP_PLATFORM_WINDOWS",
		}
		
		links
		{
			"Opengl32.lib"
		}

		files
		{
			"Platform/Windows/**.h",
			"Platform/Windows/**.hpp",
			"Platform/Windows/**.cpp"
		}

	filter "system:linux"
		cppdialect "C++23"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"BP_PLATFORM_LINUX",
		}
		
		links
	{
		"GLFW",
		"X11",
		"Xrandr",
		"Xi",
		"Xcursor",
		"Xinerama",
		"GL",
		"pthread",
		"dl",
		"m"
	}

		files
		{
			"Platform/Linux/**.h",
			"Platform/Linux/**.hpp",
			"Platform/Linux/**.cpp"
		}

	filter "configurations:Debug"
		defines "BP_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "BP_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "BP_DIST"
		optimize "On"