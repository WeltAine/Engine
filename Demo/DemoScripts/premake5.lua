project "DemoScripts"
    location "."
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "On"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    buildoptions
    {
        "/utf-8",
        "/Zm200"
    }

    includedirs
    {
        "%{wks.location}/Ayin/Dependency/spdlog/include",
        "%{wks.location}/Ayin/src",
        "%{wks.location}/Ayin/Dependency",
        ".",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.Glaze}"
    }

    defines
    {
        "GLM_ENABLE_EXPERIMENTAL"
    }

    links
    {
        "Ayin"
    }

    files
    {
        "DemoScripts.h",
        "DemoScripts.cpp",
        "TetrisScripts.cpp",
        "TetrisPieces.cpp",
        "TetrisRules.cpp",
        "TetrisRender.cpp",
        "OrbitScripts.cpp"
    }

    filter "system:windows"
        systemversion "latest"
        buildoptions "/Zc:preprocessor"

        defines
        {
            "AYIN_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "AYIN_DEBUG"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "AYIN_RELEASE"
        runtime "Release"
        optimize "On"

    filter "configurations:Dist"
        defines "AYIN_DIST"
        runtime "Release"
        optimize "On"
