project "Engine"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp",
   "Vendor/stb_image/**.h", "Vendor/stb_image/**.cpp",
   "Vendor/tiny_obj_loader/tiny_obj_loader.h",
   "Vendor/flecs/**.h", "Vendor/flecs/**.c",
   "Shaders/**.vert", "Shaders/**.frag",
    "Vendor/vma/include/**.h"}

   local vulkanSDKPath = os.getenv("VULKAN_SDK")

   defines
   {
        "GLFW_INCLUDE_VULKAN",
        "GLM_FORCE_RADIANS",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE"
   }

   links { vulkanSDKPath .. "/Lib/vulkan-1.lib", "GLFW", "assimp" }

   includedirs
   {
      "Source",
      vulkanSDKPath .. "/Include",
      "Vendor/glfw/include",
      "Vendor/glm/glm",
      "Vendor/assimp/include"
   }

   targetdir ("../Binaries/" .. outputdir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }

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