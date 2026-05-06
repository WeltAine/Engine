workspace "Ayin" --对应解决方案
    architecture "x86_64"
    startproject "Ayin-Editor" --默认启动项目

    configurations --对应vs中的配置
    {
        "Debug", --调式（比如日志全开）
        "Release", --“发行”（可能关闭一部分日志）
        "Dist" --真正的发行（不会有任何日志），这个以后会用上
    }

    flags{
        "MultiProcessorCompile" --多线程编译，提升编译速度
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" --比如Debug-Windows-x64，为不同平台做准备，虽然我没这个打算就是了，不过是个好习惯


--包含相对于根文件夹（解决方案目录）的目录
--路径记录数组
IncludeDir = {}
IncludeDir["GLFW"] = "Ayin/Dependency/GLFW/include" 
IncludeDir["Glad"] = "Ayin/Dependency/glad/include"
IncludeDir["ImGui"] = "Ayin/Dependency/ImGui"
IncludeDir["glm"] = "Ayin/Dependency/glm"
IncludeDir["stb_image"] = "Ayin/Dependency/stb_image"
IncludeDir["entt"] = "Ayin/Dependency/entt/single_include"


include "Ayin/Dependency/GLFW" --引入该目录下的premake5.lua，和C++的include一样
include "Ayin/Dependency/Glad"
include "Ayin/Dependency/ImGui"











