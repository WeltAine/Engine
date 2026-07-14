project "Snake"
    location "." -- 项目文件与源码同级，方便在 Visual Studio 中展开查看完整目录
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++23"
    staticruntime "On"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir "%{wks.location}" -- 运行时工作目录固定到仓库根目录，资源路径可直接使用 assets/...

    buildoptions
    {
        "/utf-8", -- 让 MSVC 按 UTF-8 读取源码，保证中文注释和日志文本正常
        "/Zm200"
    }

    includedirs
    {
        "%{wks.location}/Ayin/Dependency/spdlog/include",
        "%{wks.location}/Ayin/src",
        "%{wks.location}/Ayin/Dependency",
        "src",
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
        "src/**.h",
        "src/**.cpp"
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
