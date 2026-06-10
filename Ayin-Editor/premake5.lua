project "Ayin-Editor"
    location "." --项目文件和premake脚本放在同一目录，与源码同级，这样VS"显示所有文件"时可以展开完整的文件夹树方便添加新文件
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++23" 
    staticruntime "On" --???
    
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}") --输出路径
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}") --中间文件路径
	debugdir "%{wks.location}" --调试工作目录固定到仓库根目录，资源路径可使用 assets/...

    buildoptions 
    {
        "/utf-8",       --保证能够使用log，文件编码时utf-8没错，但是编码器所选择的解释方式并不默认按照文本的格式来，否则 Microsoft Visual C++ (MSVC) 编译器默认会使用系统的本地代码页（如 Windows-1252）来读取它们。所以我们需要指定utf-8，否则log系统会报错
        "/Zm200" --增加编译器堆预留空间，缓解模板重型库(Glaze)导致的堆溢出
    }


    includedirs --项目包含文件
    {
        "%{wks.location}/Ayin/Dependency/spdlog/include",
        "%{wks.location}/Ayin/src",
        "%{wks.location}/Ayin/Dependency",
        "%{wks.location}/OrbitGame/src",
        "src",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.Glaze}",
        "%{IncludeDir.ImGuizmo}"
    }

    defines{
        "GLM_ENABLE_EXPERIMENTAL"
    }

    -- dependson "Ayin"
    links --连接.lib
    {
        "Ayin"
    }


    files --构建时包含的文件
    {
        "src/**.h",
        "src/**.cpp",
        "%{wks.location}/OrbitGame/src/OrbitGame.h",
        "%{wks.location}/OrbitGame/src/OrbitGame.cpp"
    }


    filter "system:windows" --当在windows系统下构建该项目时
        systemversion "latest" --WindowsSDK，我的vs里是类似10.0.x

        buildoptions "/Zc:preprocessor" --启用预处理器标准模式，解决预编译头文件中使用__VA_OPT__时出现的错误

        defines --宏
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
