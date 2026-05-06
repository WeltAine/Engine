project "Ayin-Editor"
    location "Ayin-Editor" --？？？默认目录么，可是后头的构建包含文件和头文件为什么用了更完整的路径？？
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20" 
    staticruntime "On" --???
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}") --输出路径
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}") --中间文件路径

    buildoptions "/utf-8"


    includedirs --项目包含文件
    {
        "Ayin/Dependency/spdlog/include",
        "Ayin/src",
        "Ayin/Dependency",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}"
    }

    -- dependson "Ayin"
    links --连接.lib
    {
        "Ayin"
    }


    files --构建时包含的文件
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }


    filter "system:windows" --当在windows系统下构建该项目时
        systemversion "latest" --WindowsSDK，我的vs里是类似10.0.x


        defines --宏
        {
            "AYIN_PLATFORM_WINDOWS"
        }



    filter "configurations:Debug" --暂时没用，但先写着
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
