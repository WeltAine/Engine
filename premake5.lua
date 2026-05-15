include "./Dependency/premake/premake_customization/solution_items.lua" --引入自定义的解决方案项（比如 .editorconfig）

workspace "Ayin" --对应解决方案
    architecture "x86_64"
    startproject "Ayin-Editor" --默认启动项目

    configurations --对应vs中的配置
    {
        "Debug", --调式（比如日志全开）
        "Release", --“发行”（可能关闭一部分日志）
        "Dist" --真正的发行（不会有任何日志），这个以后会用上
    }

    solution_items --解决方案包含的文件
    {
        ".editorconfig"
    }

    multiprocessorcompile "On" --开启多线程编译，提升编译速度

-- 全局变量
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" --比如Debug-Windows-x64，为不同平台做准备，虽然我没这个打算就是了，不过是个好习惯
-- %{...} 是 Premake 的令牌替换语法，在生成工程文件时展开为实际值。

--包含相对于根文件夹（解决方案目录）的目录
--路径记录数组
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Ayin/Dependency/GLFW/include" 
IncludeDir["Glad"] = "%{wks.location}/Ayin/Dependency/glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Ayin/Dependency/ImGui"
IncludeDir["glm"] = "%{wks.location}/Ayin/Dependency/glm"
IncludeDir["stb_image"] = "%{wks.location}/Ayin/Dependency/stb_image"
IncludeDir["entt"] = "%{wks.location}/Ayin/Dependency/entt/single_include"
IncludeDir["Glaze"] = "%{wks.location}/Ayin/Dependency/Glaze/include"

group "Dependency" --分组，和vs中的过滤器类似
    include "Ayin/Dependency/GLFW" --引入该目录下的premake5.lua，和C++的include一样
    include "Ayin/Dependency/Glad"
    include "Ayin/Dependency/ImGui"
group "" --结束分组，回到默认分组

include "Ayin" --引入Ayin项目
include "Ayin-Editor" --引入Ayin-Editor项目
-- include "SandBox" --引入SandBox项目










