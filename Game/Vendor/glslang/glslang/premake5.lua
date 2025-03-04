project "glslang"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "pch.h"
    pchsource "MachineIndependent/pch.cpp"

    forceincludes  
    {
        "pch.h"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "ENABLE_SPIRV",
        "ENABLE_HLSL",
        "ENABLE_OPT=1",
    }

    files
    {
        "CInterface/**.cpp",
        "GenericCodeGen/**.cpp",
        "HLSL/**.cpp",
        "HLSL/**.h",
        "Include/**.h",
        "MachineIndependent/**.h",
        "MachineIndependent/**.cpp",
        "ResourceLimits/**.h",
        "ResourceLimits/**.cpp",
        "Public/**.h",
        "../SPIRV/**.cpp",
        "../SPIRV/**.h",

        "OSDependent/osinclude.h"
    }

    includedirs
    {
        "",
        "../",
        "Include",

        "../External",
        "../External/spirv-tools/include",
        "../External/spirv-tools/external/spirv-headers/include",
    }

    links{
        "SPIRV-Tools-opt",
        "SPIRV-Tools-static"
    }
    
    filter "system:windows"
        systemversion "latest"
        defines { "GLSLLANG_OSINCLUDE_WIN32" }
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
