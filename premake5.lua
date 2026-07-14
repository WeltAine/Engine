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

    solution_items --解决方案包含的文件，会在 VS 的 Solution Items 文件夹中显示
    {
        ".editorconfig",
        ".gitattributes",
        ".gitignore",
        "Demo/TetrisGame/TetrisScene.json",
        "Demo/OrbitGame/OrbitScene.json",
        "Demo/Tutorial/TutorialScene.json"
    }

    multiprocessorcompile "On" --开启多线程编译，提升编译速度

-- 全局变量
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" --比如Debug-Windows-x64，为不同平台做准备，虽然我没这个打算就是了，不过是个好习惯
-- %{...} 是 Premake 的令牌替换语法，在生成工程文件时展开为实际值。

--包含相对于根文件夹（解决方案目录）的目录
--路径记录数组
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Ayin/Dependency/GLFW/include"                                                 -- GLFW，一个跨平台的窗口和输入库，提供了创建窗口、处理输入等功能，适用于图形应用程序开发。
IncludeDir["Glad"] = "%{wks.location}/Ayin/Dependency/glad/include"                                                 -- Glad，一个OpenGL加载库，负责加载OpenGL函数指针，使得开发者可以使用OpenGL的功能。
IncludeDir["ImGui"] = "%{wks.location}/Ayin/Dependency/ImGui"                                                       -- ImGui，一个即时模式图形用户界面库，提供了丰富的UI组件和工具，适用于游戏开发和图形应用程序。
IncludeDir["glm"] = "%{wks.location}/Ayin/Dependency/glm"                                                           -- GLM，一个基于C++的数学库，提供了向量、矩阵等数学功能，适用于图形编程和游戏开发。
IncludeDir["stb_image"] = "%{wks.location}/Ayin/Dependency/stb_image"                                               -- stb_image，一个单文件的图像加载库，支持多种图像格式，适用于图形应用程序开发。
IncludeDir["entt"] = "%{wks.location}/Ayin/Dependency/entt/single_include"                                          -- EnTT，一个现代C++的实体组件系统（ECS）库，提供了高效的实体管理和组件系统，适用于游戏开发和复杂应用程序。
IncludeDir["Glaze"] = "%{wks.location}/Ayin/Dependency/Glaze/include"                                               -- Glaze，一个现代C++的序列化库，提供了高效的对象序列化和反序列化功能，适用于游戏开发和数据持久化。
IncludeDir["NativeFileDialogExtended"] = "%{wks.location}/Ayin/Dependency/nativefiledialog-extended/src/include"    -- NativeFileDialogExtended，一个跨平台的文件对话框库，提供了在不同操作系统上显示文件选择对话框的功能，适用于图形应用程序开发。
IncludeDir["ImGuizmo"] = "%{wks.location}/Ayin/Dependency/ImGuizmo/src"                                                 -- ImGuizmo，一个基于ImGui的3D变换工具库，提供了在图形应用程序中进行对象变换的功能，适用于游戏开发和图形编辑器。


group "Dependency" --分组，和vs中的过滤器类似
    include "Ayin/Dependency/GLFW" --引入该目录下的premake5.lua，和C++的include一样
    include "Ayin/Dependency/Glad"
    include "Ayin/Dependency/ImGui"
    include "Ayin/Dependency/nativefiledialog-extended"
group "" --结束分组，回到默认分组

include "Ayin" --引入Ayin项目

group "Demo"
    include "Demo/DemoScripts"
    include "Demo/Snake"
    include "Demo/TetrisGame"
    include "Demo/OrbitGame"
group ""

include "Ayin-Editor" --引入Ayin-Editor项目
-- include "SandBox" --引入SandBox项目







