workspace "BitPounce"
    architecture "x64"

    configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Global defaults
filter {}
    defines { "FMT_UNICODE=0", "remainderf(x,y)=((float)remainder((x),(y)))" }

-- Platform-specific
filter "system:windows"
    staticruntime "Off"
    cdialect "C17"
filter {}

-- Groups
group "Core"
    include "BitPounce"
    include "PounceEdit"
group ""

include "Sandbox"

group "Dependencies"
    include "box2d.lua"
    include "BitPounce/vendor/GLFW"
    include "BitPounce/vendor/imgui"
    include "BitPounce/vendor/BitPouncePack"

    group "Dependencies/Font"
        include "BitPounce/vendor/msdf-atlas-gen"
    group "Dependencies"

group ""