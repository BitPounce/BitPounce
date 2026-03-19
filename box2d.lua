project "Box2D"
    location "build/Box2D"
    kind "StaticLib"
    language "C"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "BitPounce/vendor/box2d/src/**.c",
        "BitPounce/vendor/box2d/include/**.h"
    }

    includedirs {
        "BitPounce/vendor/box2d/include"
    }