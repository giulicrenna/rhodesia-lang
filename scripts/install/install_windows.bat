@echo off

:: Rhodesia Language Installer for Windows
:: This script installs the Rhodesia programming language and its dependencies

:: Check if running as administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo This script requires administrator privileges.
    echo Please run as administrator.
    pause
    exit /b 1
)

:: Install required dependencies using Chocolatey
echo Checking for Chocolatey...
where choco >nul 2>&1
if %errorLevel% neq 0 (
    echo Installing Chocolatey package manager...
    @"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -InputFormat None -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))" && SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"

    :: Refresh environment variables
    call refreshenv
)

echo Installing required dependencies...
choco install -y cmake git llvm make --no-progress

:: Clone Rhodesia repository if not already present
if not exist "C:\rhodesia-lang" (
    echo Cloning Rhodesia repository...
    git clone https://github.com/giulicrenna/rhodesia-lang.git C:\rhodesia-lang
) else (
    echo Rhodesia repository already exists at C:\rhodesia-lang
)

:: Build the project
echo Building Rhodesia...
cd C:\rhodesia-lang
if not exist "build" mkdir build
cd build
cmake ..
cmake --build . --config Release

:: Add Rhodesia to PATH
setx /M PATH "%PATH%;C:\rhodesia-lang\build"

:: Create a shortcut for easy access (optional)
echo Creating desktop shortcut...
set SCRIPT="%TEMP%\rhodesia_shortcut.vbs"
echo Set oWS = WScript.CreateObject("WScript.Shell") > %SCRIPT%
echo sLinkFile = "%USERPROFILE%\Desktop\Rhodesia.lnk" >> %SCRIPT%
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> %SCRIPT%
echo oLink.TargetPath = "C:\rhodesia-lang\build\rhodesia.exe" >> %SCRIPT%
echo oLink.WorkingDirectory = "C:\rhodesia-lang\build" >> %SCRIPT%
echo oLink.Description = "Rhodesia Programming Language" >> %SCRIPT%
echo oLink.Save >> %SCRIPT%
cscript /nologo %SCRIPT%
del %SCRIPT%

echo.
echo Rhodesia installation completed successfully!
echo You can now run 'rhodesia' from anywhere in Command Prompt.
echo Try running: rhodesia examples\basic\hello.rho
pause
