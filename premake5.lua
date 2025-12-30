workspace "BitPounce"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

defines {
	"FMT_UNICODE=0"
}

include "Sandbox"
include "BitPounce"
include "BitPounce/vendor/GLFW"
include "BitPounce/vendor/imgui"