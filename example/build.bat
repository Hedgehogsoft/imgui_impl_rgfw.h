@echo off
setlocal enabledelayedexpansion
cd /D "%~dp0"

:: Derived from RADDBG build.bat

:: --- Apply Build Environment -------------------------------------------------
if "%PROCESSOR_ARCHITECTURE%" equ "AMD64" (
    set ARCH=x64
) else if "%PROCESSOR_ARCHITECTURE%" equ "ARM64" (
    set ARCH=arm64
)

where /Q cl.exe || (
    echo [apply build environment]
    set __VSCMD_ARG_NO_LOGO=1
    for /f "tokens=*" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop -property installationPath') do set VS=%%i
    if "!VS!" equ "" (
        echo [ERROR] Visual Studio installation not found
        exit /b 1
    )
    call "!VS!\Common7\Tools\VsDevCmd.bat" -arch=%ARCH% -host_arch=%ARCH% -startdir=none -no_logo || exit /b 1
)

:: --- Unpack Arguments --------------------------------------------------------
for %%a in (%*) do set "%%~a=1"
if not "%msvc%"=="1" if not "%clang%"=="1" set msvc=1
if not "%debug%"=="1" set release=1
if "%debug%"=="1" set release=0 && echo [debug mode]
if "%release%"=="1" set debug=0 && echo [release mode]
if "%msvc%"=="1"    set clang=0 && echo [msvc compile]
if "%clang%"=="1"   set msvc=0 && echo [clang compile]

set compile_flags=
if "%docking%"=="1" set compile_flags=%compile_flags% -DRGFW_IMGUI_DOCKING_EXAMPLE && echo [docking enabled]

:: --- Compile Definitions -----------------------------------------------------
set cl_include=    /Iimgui\ /I.
set cl_common=     /nologo /FC /Z7 /W4 /std:c++17 /EHsc /Zc:__cplusplus %cl_include% %compile_flags%
set cl_debug=      call cl /Od /MTd /D_DEBUG /RTC1 %cl_common%
set cl_release=    call cl /O2 /MT /DNDEBUG %cl_common%
set cl_link=       /link /MANIFEST:EMBED /INCREMENTAL:NO /noexp
set cl_out=        /out:
set clang_include= -Iimgui\ -I.
set clang_common=  -fdiagnostics-absolute-paths -Wall -Wextra -std=c++17 -Dgnu_printf=printf %clang_include%
set clang_debug=   call clang++ -g -gcodeview -O0 -D_DEBUG -fms-runtime-lib=static_dbg %clang_common% %compile_flags%
set clang_release= call clang++ -g -gcodeview -O2 -DNDEBUG -fms-runtime-lib=static %clang_common% %compile_flags%
set clang_link=    -fuse-ld=lld -Wl,/MANIFEST:EMBED
set clang_out=     -o

:: --- Choose Compile/Link Lines ----------------------------------------------
if "%msvc%"=="1"    set compile_debug=%cl_debug%
if "%msvc%"=="1"    set compile_release=%cl_release%
if "%msvc%"=="1"    set compile_link=%cl_link%
if "%msvc%"=="1"    set out=%cl_out%
if "%msvc%"=="1"    set only_compile=/c
if "%msvc%"=="1"    set obj_out=%cl_obj_out%
if "%msvc%"=="1"    set obj_ext=.obj
if "%clang%"=="1"   set compile_debug=%clang_debug%
if "%clang%"=="1"   set compile_release=%clang_release%
if "%clang%"=="1"   set compile_link=%clang_link%
if "%clang%"=="1"   set out=%clang_out%
if "%clang%"=="1"   set only_compile=-c
if "%clang%"=="1"   set obj_out=%clang_obj_out%
if "%clang%"=="1"   set obj_ext=.o
if "%debug%"=="1"   set compile=%compile_debug%
if "%release%"=="1" set compile=%compile_release%

:: --- Build -------------------------------------------------------------------
if not exist imgui_lib%obj_ext% (
    echo [building imgui]
    %compile% %only_compile% imgui\*.cpp || exit /b 1
    lib /nologo /out:imgui_lib%obj_ext% *%obj_ext% || exit /b 1
    for %%f in (*%obj_ext%) do if not "%%f"=="imgui_lib%obj_ext%" del "%%f"
)

%compile% main.cpp imgui_lib%obj_ext% %compile_link% %out%rgfw-imgui.exe || exit /b 1

:: MSVC does not clean up one line compile artifacts unlike Clang.
if "%msvc%"=="1" del main%obj_ext% >nul

:: --- Run On Debug ------------------------------------------------------------
if "%debug%"=="1" if exist rgfw-imgui.exe (
    rgfw-imgui.exe
)
