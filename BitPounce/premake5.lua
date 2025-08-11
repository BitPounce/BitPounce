project "BitPounce"
	kind "StaticLib"
	language "C++"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"src"
	}
	
	links
	{
		
	}

	filter "system:windows"
		cppdialect "C++23"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"BP_PLATFORM_WINDOWS",
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