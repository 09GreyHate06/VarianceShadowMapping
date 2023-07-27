workspace "VarianceShadowMapping"
    architecture "x86_64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GreyDX11"]  = "Sandbox/vendor/GreyDX11/GreyDX11"
IncludeDir["ImGui"]     = "Sandbox/vendor/imgui"
IncludeDir["assimp"]    = "Sandbox/vendor/assimp/include"

include "Sandbox/vendor/GreyDX11"
include "Sandbox/vendor/imgui"
include "Sandbox/vendor/assimp"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/res/shaders/**.hlsl",
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{IncludeDir.GreyDX11}/src",
        "%{IncludeDir.GreyDX11}/vendor/stb_image",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.GreyDX11}/vendor/spdlog/include",
        "%{IncludeDir.assimp}"
    }

    links
    {
        "GreyDX11",
        "imgui",
        "assimp",
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "GDX11_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "GDX11_RELEASE"
        runtime "Release"
        optimize "on"

    filter "files:**.hlsl"
        shaderobjectfileoutput "res/cso/%{file.basename}.cso"
        removeflags "ExcludeFromBuild"
        shadermodel "5.0"
        shaderentry "main"

    filter "files:**.vs.hlsl"
        shadertype "Vertex"

    filter "files:**.ps.hlsl"
        shadertype "Pixel"

    filter "files:**.gs.hlsl"
        shadertype "Geometry"