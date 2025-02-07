-- premake5.lua
workspace "Stealth Game"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Game"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

outputdir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"
shaderdir = outputdir .. "/shaders"

group "Dependencies"
    include "Game/Vendor/glfw"
    include "Game/Vendor/assimp"
    include "Game/Vendor/imgui-premake"
group ""

include "Dependencies.lua"
include "Game/Build-Game.lua"
