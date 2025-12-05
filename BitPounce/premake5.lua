project "BitPounce"
	kind "StaticLib"
	language "C++"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "bp_pch.h"
	pchsource "src/bp_pch.cpp"

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
		
		"Platform/Windows/**.h",
		"Platform/Windows/**.hpp",
		"Platform/Windows/**.cpp"
	}

	includedirs
	{
		"src",
		"../BitPounce/vendor/spdlog/include",
		"vendor/GLFW/include",
		"./"
	}
	
	links
	{
		"GLFW"
	}

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

	filter "configurations:Debug"
		defines "BP_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "BP_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "BP_DIST"
		optimize "On"