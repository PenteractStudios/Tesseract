@echo off
:: --HAS ENDING BACKSLASH 
set batdir=%~dp0 
:: --MISSING ENDING BACKSLASH 
:: set batdir=%CD% 
pushd "%batdir%"

for %%i in ("%~dp0.") do set "folder=%%~fi"

set solutionDir=%folder%\..\Penteract.sln
echo %solutionDir%

if "%VSWHERE%"=="" set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
  set InstallDir=%%i
)

call "%InstallDir%\VC\Auxiliary\Build\vcvarsall.bat" x86

devenv "%solutionDir%" /Clean Release
devenv "%solutionDir%" /Build Release

exit /b