@echo off
if NOT exist winbuild-msvc.bat exit /b 255

if "%VSCMD_ARG_HOST_ARCH%"=="" (
echo Run this batch file from Developer Command Prompt for VS 20XX
exit /b 255
)

set INSTALLDIR=octcalc_install

if %VSCMD_ARG_HOST_ARCH%==x86 call "%VCINSTALLDIR%Auxiliary\Build\vcvars64.bat"

if exist build\ rmdir /S /Q build
mkdir build
cd build
if exist ..\..\%INSTALLDIR%\ rmdir /S /Q ..\..\%INSTALLDIR%
cmake .. -GNinja -DCMAKE_INSTALL_PREFIX=../../%INSTALLDIR% %*
if not errorlevel 0 exit /b 255
cmake -E time ninja
if not errorlevel 0 exit /b 255
if exist CPackConfig.cmake (
cpack -G WIX
if not errorlevel 0 exit /b 255
if exist ..\..\sign.bat (
for /f %%f in ('dir /b *.msi') do ..\..\sign.bat %%f
)
) else (
ninja install
)
