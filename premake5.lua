workspace "Ayin" --对应解决方案
    architecture "x64"
    startproject "SandBox" --默认启动项目

    configurations --对应vs中的配置
    {
        "Debug", --调式（比如日志全开）
        "Release", --“发行”（可能关闭一部分日志）
        "Dist" --真正的发行（不会有任何日志），这个以后会用上
    }


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" --比如Debug-Windows-x64，为不同平台做准备，虽然我没这个打算就是了，不过是个好习惯


--包含相对于根文件夹（解决方案目录）的目录
--路径记录数组
IncludeDir = {}
IncludeDir["GLFW"] = "Ayin/Dependency/GLFW/include" 
IncludeDir["Glad"] = "Ayin/Dependency/glad/include"
IncludeDir["ImGui"] = "Ayin/Dependency/ImGui"
IncludeDir["glm"] = "Ayin/Dependency/glm"


include "Ayin/Dependency/GLFW" --引入该目录下的premake5.lua，和C++的include一样
include "Ayin/Dependency/Glad"
include "Ayin/Dependency/ImGui"


project "Ayin"
    location "Ayin" --？？？默认目录么，可是后头的构建包含文件和头文件为什么用了更完整的路径？？
    kind "SharedLib" --生成动态库
    language "C++"
    staticruntime "off" --动态连接库
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}") --输出路径
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}") --中间文件路径

    pchheader "AyinPch.h" --Use Precompiled Header

    buildoptions "/utf-8" --保证能够使用log，文件编码时utf-8没错，但是编码器所选择的解释方式并不默认按照文本的格式来，否则 Microsoft Visual C++ (MSVC) 编译器默认会使用系统的本地代码页（如 Windows-1252）来读取它们。所以我们需要指定utf-8，否则log系统会报错

    includedirs --项目包含文件
    {
        "%{prj.name}/src", --Ayin文件的根目录，为更深层次目录中的文件include时提供方便
        "%{prj.name}/Dependency/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}"
    }

    -- 依赖项目
    links{
        "GLFW",
        "Glad",
        "ImGui",
        "opengl32.lib"
    }


    files --构建时包含的文件
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/Dependency/glm/glm/**.hpp",
        "%{prj.name}/Dependency/glm/glm/**.inl"
    }


    filter "system:windows" --当在windows系统下构建该项目时
        cppdialect "C++20" --C++方言，哈哈哈哈
        systemversion "latest" --WindowsSDK，我的vs里是类似10.0.x

        pchsource "Ayin/src/AyinPch.cpp" -- Creat Precompiled Header


        defines --宏
        {
            "AYIN_PLATFORM_WINDOWS",
            "AYIN_BUILD_DLL",
            "GLFW_INCLUDE_NONE" --让glfw不包含OpenGL头文件，因为我们用的是Glad来加载OpenGL函数，而且由glfw引入头文件会报错（不理解为什么要这么处理）
        }


        postbuildcommands --转移dll，
        {
            ("{MKDIR} ../bin/" .. outputdir .. "/SandBox"), --需要先构建目录再赋值，否则会出错（产生一个SandBox文件而不是文件夹），唉
            ("{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/SandBox") --自动将dll移动到SandBox的输出文件夹中
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

--注意过滤器都是独立的，尽管它们可以多个同时生效，但不具备联合的与判定效果（可以使用filters）


project "SandBox"
    location "SandBox" --？？？默认目录么，可是后头的构建包含文件和头文件为什么用了更完整的路径？？
    kind "ConsoleApp" --生成动态库
    language "C++"
    staticruntime "off" --动态连接库
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}") --输出路径
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}") --中间文件路径

    buildoptions "/utf-8"


    includedirs --项目包含文件
    {
        "Ayin/Dependency/spdlog/include",
        "Ayin/src",
        "%{IncludeDir.glm}"

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
        cppdialect "C++20" --C++方言，哈哈哈哈
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
