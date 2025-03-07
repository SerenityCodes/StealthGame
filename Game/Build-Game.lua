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
    "Source/FileIO/**.h", "Source/FileIO/**.cpp",

    "Vendor/concurrentqueue/**.h",
    "Vendor/doctest/doctest/**.h", "Vendor/doctest/doctest/**.cpp",
    "Vendor/flecs/**.h", "Vendor/flecs/**.c",
    "Vendor/fmt/src/**.cc",
    "Vendor/imgui/**.h", "Vendor/imgui/**.cpp",
    "Vendor/ImGuizmo/**.cpp", "Vendor/ImGuizmo/**.h",
    "Vendor/spdlog/include/**.h", "Vendor/spdlog/src/**.cpp",
    "Vendor/stb_image/**.h", "Vendor/stb_image/**.cpp",
    "Vendor/glm/**.hpp", "Vendor/glm/**.h",
    "Vendor/vma/include/**.h",
    "Vendor/SPIRV-Cross/**.h", "Vendor/SPIRV-Cross/**.hpp", "Vendor/SPIRV-Cross/**.cpp",
    "Vendor/vk-bootstrap/**.h", "Vendor/vk-bootstrap/**.cpp",
    "Vendor/vuk/src/**.cpp", "Vendor/vuk/src/**.hpp"}

   defines
   {
       "GLFW_INCLUDE_VULKAN",
       "GLM_FORCE_RADIANS",
       "GLM_FORCE_DEPTH_ZERO_TO_ONE",
       "SPDLOG_COMPILED_LIB",
       "NOMINMAX",
       "VC_EXTRALEAN",
        "WIN32_LEAN_AND_MEAN",
        "_CRT_SECURE_NO_WARNINGS",
        "_SCL_SECURE_NO_WARNINGS",
        "_SILENCE_CLANG_CONCEPTS_MESSAGE",
        "_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING",
        "SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS",
   }

   local vulkanSDKPath = os.getenv("VULKAN_SDK")

   includedirs
   {
      "Source",
	  vulkanSDKPath .. "/Include",
	  "Vendor/concurrentqueue/",
	  "Vendor/doctest",
	  "Vendor/fmt/include",
      "Vendor/glfw/include",
      "Vendor/glm/glm",
      "Vendor/imgui",
      "Vendor/ImGuizmo",
      "Vendor/plf_colony",
      "Vendor/robin-hood-hashing",
      "Vendor/spdlog/include",
      "Vendor/SPIRV-Cross",
      "Vendor/SPIRV-Cross/include",
      "Vendor/stb",
      "Vendor/vma/include",
      "Vendor/assimp/include",
      "Vendor/vuk/include",
      "Vendor/vk-bootstrap"
   }

   links
   {
      vulkanSDKPath .. "/Lib/vulkan-1.lib",
      "GLFW", "assimp"
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
       defines { "RELEASE", "NDEBUG" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST", "NDEBUG" }
       runtime "Release"
       optimize "On"
       symbols "Off"