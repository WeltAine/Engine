workspace "Ayin" --对应解决方案
    architecture "x64"

    configurations --对应vs中的配置
    {
        "Debug", --调式（比如日志全开）
        "Release", --“发行”（可能关闭一部分日志）
        "Dist" --真正的发行（不会有任何日志），这个以后会用上
    }


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" --比如Debug-Windows-x64，为不同平台做准备，虽然我没这个打算就是了，不过是个好习惯


project "Ayin"
    location "Ayin" --？？？默认目录么，可是后头的构建包含文件和头文件为什么用了更完整的路径？？
    kind "SharedLib" --生成动态库
    language "C++"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}") --输出路径
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}") --中间文件路径

    buildoptions "/utf-8" --保证能够使用log

    includedirs --项目包含文件
    {
        "%{prj.name}/Dependency/spdlog/include"
    }


    files --构建时包含的文件
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }


    filter "system:windows" --当在windows系统下构建该项目时
        cppdialect "C++20" --C++方言，哈哈哈哈
        staticruntime "On" --静态连接库
        systemversion "latest" --WindowsSDK，我的vs里是类似10.0.x


        defines --宏
        {
            "AYIN_PLATFORM_WINDOWS",
            "AYIN_BUILD_DLL"
        }


        postbuildcommands --转移dll，
        {
            ("{MKDIR} ../bin/" .. outputdir .. "/SandBox"), --需要先构建目录再赋值，否则会出错（产生一个SandBox文件而不是文件夹），唉
            ("{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/SandBox") --自动将dll移动到SandBox的输出文件夹中
        }

    filter "configurations:Debug" --暂时没用，但先写着
        defines "AYIN_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "AYIN_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "AYIN_DIST"
        optimize "On"

--注意过滤器都是独立的，尽管它们可以多个同时生效，但不具备联合的与判定效果（可以使用filters）


project "SandBox"
    location "Sandbox" --？？？默认目录么，可是后头的构建包含文件和头文件为什么用了更完整的路径？？
    kind "ConsoleApp" --生成动态库
    language "C++"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}") --输出路径
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}") --中间文件路径

    buildoptions "/utf-8"


    includedirs --项目包含文件
    {
        "Ayin/Dependency/spdlog/include",
        "Ayin/src"
    }

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
        cppdialect "C++20" --C++方言，哈哈哈哈
        staticruntime "On" --静态连接库
        systemversion "latest" --WindowsSDK，我的vs里是类似10.0.x


        defines --宏
        {
            "AYIN_PLATFORM_WINDOWS"
        }



    filter "configurations:Debug" --暂时没用，但先写着
        defines "AYIN_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "AYIN_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "AYIN_DIST"
        optimize "On"
