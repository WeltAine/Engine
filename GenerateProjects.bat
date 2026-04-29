@echo off
chcp 65001 >nul
cd /d "%~dp0"

echo ========================================
echo 正在清理旧的生成文件...
echo ========================================


:: 删除输出目录
if exist "bin" (
    rd /s /q "bin"
    echo [√] 已删除 bin 文件夹
)

:: 删除中间文件目录
if exist "bin-int" (
    rd /s /q "bin-int"
    echo [√] 已删除 bin-int 文件夹
)

:: 删除解决方案文件
if exist "*.sln" (
    del /q "*.sln"
    echo [√] 已删除解决方案文件
)

:: 删除各项目的 VS 项目文件
for %%p in (Ayin, Ayin-Editor, SandBox) do (
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