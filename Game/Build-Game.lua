project "Game"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/Components/**.h", "Source/Components/**.cpp",
    "Source/Containers/**.h", "Source/Containers/**.cpp",
    "Source/Logging/**.h", "Source/Logging/**.cpp",
    "Source/Memory/**.h", "Source/Memory/**.cpp",
    "Source/Systems/**.h", "Source/Systems/**.cpp",
    "Source/Vulkan/**.h", "Source/Vulkan/**.cpp",
    "Source/App.cpp", "Source/common.h", "Source/Engine*",
    "Vendor/flecs/**.h", "Vendor/flecs/**.c",
    "Vendor/spdlog/include/**.h", "Vendor/spdlog/src/**.cpp",
    "Vendor/stb_image/**.h", "Vendor/stb_image/**.cpp",
    "Vendor/glm/**.hpp", "Vendor/glm/**.h",
    "Vendor/imgui/**.h", "Vendor/imgui/**.cpp",
    "Vendor/vma/include/**.h"}

   defines
   {
       "GLFW_INCLUDE_VULKAN",
       "GLM_FORCE_RADIANS",
       "GLM_FORCE_DEPTH_ZERO_TO_ONE",
       "SPDLOG_COMPILED_LIB"
   }

   local vulkanSDKPath = os.getenv("VULKAN_SDK")

   includedirs
   {
      "Source",
	  vulkanSDKPath .. "/Include",
      "Vendor/glfw/include",
      "Vendor/glm/glm",
      "Vendor/imgui",
      "Vendor/vma/include",
      "Vendor/spdlog/include",
      "Vendor/assimp/include"
   }

   links
   {
      vulkanSDKPath .. "/Lib/vulkan-1.lib", "GLFW", "assimp"
   }

   targetdir ("../Binaries/" .. outputdir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   filter "toolset:msc*"
       buildoptions { "/utf-8", "/FS" }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"