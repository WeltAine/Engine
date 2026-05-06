project "Ayin"
    location "Ayin" --？？？默认目录么，可是后头的构建包含文件和头文件为什么用了更完整的路径？？
    kind "StaticLib" --生成静态库
    language "C++"
    cppdialect "C++20"
    staticruntime "On" --???
    
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


    files --project构建时包含的文件
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/Dependency/glm/glm/**.hpp",
        "%{prj.name}/Dependency/glm/glm/**.inl",
        "%{prj.name}/Dependency/stb_image/**.h",
        "%{prj.name}/Dependency/stb_image/**.cpp"

    }


    filter "system:windows" --当在windows系统下构建该项目时
        systemversion "latest" --WindowsSDK，我的vs里是类似10.0.x
        pchsource "Ayin/src/AyinPch.cpp" -- Creat Precompiled Header


        defines --宏
        {
            "AYIN_PLATFORM_WINDOWS",
            "AYIN_BUILD_DLL", --这个宏是为了区分是构建Ayin库还是使用Ayin库，虽然现在Ayin是静态库，但以后可能会改成动态库，所以先写着
            "GLFW_INCLUDE_NONE", --让glfw不包含OpenGL头文件，因为我们用的是Glad来加载OpenGL函数，而且由glfw引入头文件会报错（不理解为什么要这么处理）
            "_CRT_SECURE_NO_WARNINGS"
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