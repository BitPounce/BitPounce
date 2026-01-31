function exposeBitPounceDeps()

    links { "GLFW", "ImGui" }

    -- Include BitPounce headers
    includedirs
    {
        "../BitPounce/src",
        "../BitPounce/vendor/spdlog/include",
        "../BitPounce/vendor/glm",
        "../BitPounce/vendor/imgui",
        "../BitPounce/vendor/glad/include",
        "../BitPounce/vendor/debugbreak"
    }

    -- Platform-specific libraries
    filter "system:windows"
        links { "GLFW", "ImGui", "Opengl32.lib" }

    filter "system:linux"
        links
        {
            "GLFW",
            "ImGui",
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

		libdirs
    	{
    	     "../BitPounce/vendor/bgfx/.build/linux64_gcc/bin"
    	}

	
filter "system:Bindoj"
    libdirs
    {
        "../BitPounce/vendor/bgfx/.build/win64_vs2022/bin"
    }

filter { "configurations:Debug", "system:linux or system:Bindoj" }
    links
    {
        "bxDebug",
        "bimg_decodeDebug",
        "bgfxDebug"
    }

filter {}


    filter {} -- clear filter
end

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
		"src/**.c",

		"Platform/OpenGL/**.h",
		"Platform/OpenGL/**.hpp",
		"Platform/OpenGL/**.cpp",

		"../BitPounce/vendor/glad/src/gl.c",

		"vendor/imgui/backends/imgui_impl_opengl3.cpp",
		"vendor/imgui/backends/imgui_impl_glfw.cpp"
	}

	includedirs
	{
		"src",
		"../BitPounce/vendor/spdlog/include",
		"../BitPounce/vendor/glad/include",
		"vendor/GLFW/include",
		"./",
		"vendor/imgui",
		"vendor/glm",
		"vendor/debugbreak",
		"vendor/stb"
	}

	links
	{
		"GLFW",
		"ImGui"
	}

	exposeBitPounceDeps()
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
		"m",
		"ImGui"
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

