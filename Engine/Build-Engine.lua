project "Engine"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp", "Vendor/stb_image/**.h", "Vendor/stb_image/**.cpp", "Shaders/**.vert", "Shaders/**.frag" }

   local vulkanSDKPath = os.getenv("VULKAN_SDK")

   defines
   {
        "GLFW_INCLUDE_VULKAN",
        "GLM_FORCE_RADIANS",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE"
   }

   links { vulkanSDKPath .. "/Lib/vulkan-1.lib", "GLFW" }

   includedirs
   {
      "Source",
      vulkanSDKPath .. "/Include",
      "Vendor/glfw/include",
      "Vendor/glm/glm"
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

   filter { "files:**.vert" }
      flags "ExcludeFromBuild"
      shadermodel "5.0"
   filter { "files:**.frag" }
      removeflags "ExcludeFromBuild"
      shadertype "Fragment"
      shaderentry "ForFragment"
    filter {}
