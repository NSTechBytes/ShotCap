@echo off
setlocal

:: Set paths
set "NSIS_PATH=C:\Program Files (x86)\NSIS\makensis.exe"
set "NSI_SCRIPT=ShotCapInstaller.nsi"
set "LOG_FILE=nsis_build_log.txt"

:: Change to the script directory
cd /d "%~dp0"

:: Log start time
echo [INFO] Build started at %date% %time% > "%LOG_FILE%"
echo [INFO] Using NSIS compiler at: %NSIS_PATH% >> "%LOG_FILE%"
echo [INFO] Compiling script: %NSI_SCRIPT% >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

:: Compile and redirect output to log
"%NSIS_PATH%" "%NSI_SCRIPT%" >> "%LOG_FILE%" 2>&1

:: Check for errorlevel
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed with error code %errorlevel%. See %LOG_FILE% for details.
    echo [ERROR] Compilation failed at %date% %time% >> "%LOG_FILE%"
    exit /b %errorlevel%
) else (
    echo [SUCCESS] Compilation completed successfully.
    echo [SUCCESS] Build completed at %date% %time% >> "%LOG_FILE%"
)

endlocal
pause
