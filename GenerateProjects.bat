@echo off
chcp 65001 >nul
cd /d "%~dp0"


:: 指定仅允许操作的目标文件夹 
:: 仅在这些文件夹内执行删除操作，依赖库文件夹（如Dependency）不要加入！
set "TARGET_PROJECT_FOLDERS=Ayin Ayin-Editor SandBox"
:: 排除的依赖库目录（防止误操作）
set "EXCLUDE_FOLDER=Dependency"

echo ========================================
echo 正在清理旧的生成文件（仅操作指定项目目录）...
echo 目标操作目录：%TARGET_PROJECT_FOLDERS%
echo 排除依赖库目录：%EXCLUDE_FOLDER%
echo ========================================
echo.

:: 删除根目录输出目录
if exist "bin" (
    rd /s /q "bin"
    echo [√] 已删除 bin 文件夹
)

:: 删除根目录中间文件目录
if exist "bin-int" (
    rd /s /q "bin-int"
    echo [√] 已删除 bin-int 文件夹
)

:: 删除根目录解决方案文件
if exist "*.sln" (
    del /q "*.sln"
    echo [√] 已删除解决方案文件
)

:: 删除各项目的 VS 项目文件
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

call Dependency\bin\premake\premake5.exe vs2022

echo.
pause