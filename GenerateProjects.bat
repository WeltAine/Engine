@echo off
chcp 65001 >nul
cd /d "%~dp0"


rem Allowed cleanup target folders only.
rem Delete generated files only in these folders; do not add Dependency.
set "TARGET_PROJECT_FOLDERS=Ayin Ayin-Editor SandBox Demo\DemoScripts Demo\Snake Demo\TetrisGame Demo\OrbitGame"
rem Dependency folder name excluded from cleanup.
set "EXCLUDE_FOLDER=Dependency"

echo ========================================
echo 正在清理旧的生成文件（仅操作指定项目目录）...
echo 目标操作目录：%TARGET_PROJECT_FOLDERS%
echo 排除依赖库目录：%EXCLUDE_FOLDER%
echo ========================================
echo.

rem Delete root output folder.
if exist "bin" (
    rd /s /q "bin"
    echo [√] 已删除 bin 文件夹
)

rem Delete root intermediate folder.
if exist "bin-int" (
    rd /s /q "bin-int"
    echo [√] 已删除 bin-int 文件夹
)

rem Delete root solution files.
if exist "*.sln" (
    del /q "*.sln"
    echo [√] 已删除解决方案文件
)
if exist "*.slnx" (
    del /q "*.slnx"
    echo [√] 已删除解决方案文件
)

rem Delete VS project files in each project folder.
for %%p in (%TARGET_PROJECT_FOLDERS%) do (
    if exist "%%p\*.vcxproj" (
        del /q "%%p\*.vcxproj"
        echo [√] 已删除 %%p 项目文件
    )
    if exist "%%p\*.vcxproj.filters" (
        del /q "%%p\*.vcxproj.filters"
        echo [√] 已删除 %%p 筛选器文件
    )
    if exist "%%p\*.vcxproj.user" (
        del /q "%%p\*.vcxproj.user"
        echo [√] 已删除 %%p 用户配置文件
    )
)

echo.
echo ========================================
echo 清理完成，正在生成解决方案...
echo ========================================
echo.

call Dependency\premake\bin\premake5.exe vs2026

echo.
pause
