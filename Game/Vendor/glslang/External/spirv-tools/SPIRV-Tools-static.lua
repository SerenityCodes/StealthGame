project "SPIRV-Tools-static"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"


    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "ENABLE_SPIRV",
        "ENABLE_HLSL",
        "SPIRV_CHECK_CONTEXT",
    }

    files
    {
        "include/*.h",
        "source/*.cpp",

        "include/util/**.h",
        "source/util/**.cpp",

        "include/val/**.h",
        "source/val/**.cpp",
    }

    includedirs
    {
        "",
        "../",
        "include",
        "external/spirv-headers/include",
    }

    links{}
    
    filter "system:windows"
        systemversion "latest"
        defines { 
            "GLSLLANG_OSINCLUDE_WIN32",
            "SPIRV_WINDOWS",
            "SPIRV_COLOR_TERMINAL",
        }
        includedirs { "build-windows" }
        files { "OSDependent/Windows/**.cpp"}

    filter "configurations:Debug"
        runtime "Debug"
        optimize "off"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        symbols "on"

    filter "configurations:Dist"
        runtime "Release"
        optimize "on"
        symbols "off"
