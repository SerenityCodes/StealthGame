project "glslang"
    kind "StaticLib"
    language "C++"
    staticruntime "off"
    warnings "off"
    
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	flags
	{
		"MultiProcessorCompile"
	}
	architecture "x86_64"

	filter "configurations:Debug"
		defines     "_DEBUG"

	filter "configurations:Release or Dist"
		defines     "NDEBUG"

	filter { "system:windows", "configurations:Dist", "toolset:not mingw" }
		flags		{ "LinkTimeOptimization" }
	
	filter "action:vs*"
		linkoptions {
			"/ignore:4006",
			"/ignore:4099",
			"/ignore:4251"
		}
		disablewarnings {
			"4006",
			"4099",
			"4251",
		}


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "glslang"
include "External/spirv-tools/SPIRV-Tools-static"
include "External/spirv-tools/SPIRV-Tools-opt"