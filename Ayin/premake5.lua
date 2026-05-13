project "Ayin"
    location "."        --项目文件和premake脚本放在同一目录，与源码同级，这样VS"显示所有文件"时可以展开完整的文件夹树方便添加新文件，你也可以指定
    kind "StaticLib"    --生成静态库
    language "C++"
    cppdialect "C++20"
    staticruntime "On" --???
    
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")   --输出路径
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")  --中间文件路径

    pchheader "AyinPch.h"       -- Use Precompiled Header
    pchsource "src/AyinPch.cpp" -- Create Precompiled Header

    buildoptions 
    {
        "/utf-8",       --保证能够使用log，文件编码时utf-8没错，但是编码器所选择的解释方式并不默认按照文本的格式来，否则 Microsoft Visual C++ (MSVC) 编译器默认会使用系统的本地代码页（如 Windows-1252）来读取它们。所以我们需要指定utf-8，否则log系统会报错
        "/Zc:preprocessor" --启用预处理器标准模式，解决预编译头文件中使用__VA_OPT__时出现的错误
    }

    defines --宏
    {
        --"AYIN_DENAMIC_LINK",        --开启静态链接还是动态链接
        "AYIN_BUILD_DLL",           --这个宏是为了区分是构建Ayin库还是使用Ayin库，虽然现在Ayin是静态库，但以后可能会改成动态库，所以先写着
        "GLFW_INCLUDE_NONE",        --让glfw不包含OpenGL头文件，因为我们用的是Glad来加载OpenGL函数，而且由glfw引入头文件会报错（不理解为什么要这么处理）
        "GLM_FORCE_SWIZZLE", --强制glm启用swizzle功能（比如vec4 v; v.xy()），虽然我也不清楚为什么要这么处理，但先写着
        "_CRT_SECURE_NO_WARNINGS"
    }


    includedirs --项目包含文件
    {
        "src", --Ayin文件的根目录，为更深层次目录中的文件include时提供方便
        "Dependency/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}"
    }

    -- 依赖项目
    links{
        "GLFW",
        "Glad",
        "ImGui",
        "opengl32.lib"
    }

    -- files的路径是相对于premake脚本所在目录的，在此基础上拼接完整路径
    -- includedir中也是如此
    -- 完整绝对路径不存在该问题
    -- 当写入.vcxproj时，会写入相对于.vcxproj文件的路径，所以在这里写相对于premake脚本的路径就行了（除非你locatin了指定位置）
    files --project构建时包含的文件
    {
        --%{prj.name}展开就只是项目名称
        "src/**.h",
        "src/**.cpp",
        "Dependency/glm/glm/**.hpp",
        "Dependency/glm/glm/**.inl",
        "Dependency/stb_image/**.h",
        "Dependency/stb_image/**.cpp"

    }


    filter "system:windows" --当在windows系统下构建该项目时
        systemversion "latest" --WindowsSDK，我的vs里是类似10.0.x


        defines --宏
        {
            "AYIN_PLATFORM_WINDOWS",
        }


        postbuildcommands --转移dll，
        {
            ("{MKDIR} ../bin/" .. outputdir .. "/SandBox"), --需要先构建目录再赋值，否则会出错（产生一个SandBox文件而不是文件夹），唉
            ("{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/SandBox") --自动将dll移动到SandBox的输出文件夹中
        }

    filter "configurations:Debug" --暂时没用，但先写着
        defines "AYIN_DEBUG"
        runtime "Debug" --使用调试版本的运行时库（比如msvcrtd.lib），以便在调试时能够获得更详细的错误信息和调试支持
        symbols "On" 

    filter "configurations:Release"
        defines "AYIN_RELEASE"
        runtime "Release" --使用发布版本的运行时库（比如msvcrt.lib），以获得更好的性能和较小的二进制文件
        optimize "On"

    filter "configurations:Dist"
        defines "AYIN_DIST"
        runtime "Release"
        optimize "On"

--注意过滤器都是独立的，尽管它们可以多个同时生效，但不具备联合的与判定效果（可以使用filters）