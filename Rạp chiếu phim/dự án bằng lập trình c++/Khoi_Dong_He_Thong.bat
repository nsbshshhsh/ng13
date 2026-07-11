@echo off
cd /d "%~dp0"
title CineMax - Quick Launch

echo ====================================================================
echo                 CINEMAX QUICK LAUNCH SYSTEM
echo ====================================================================
echo.

:: 1. Create Desktop Shortcut if it does not exist
set "SHORTCUT_PATH=%USERPROFILE%\Desktop\CineMax Booking.lnk"
if exist "%SHORTCUT_PATH%" goto :CHECK_SERVER

echo [INFO] Creating Desktop Shortcut...
python create_shortcut.py >nul 2>&1
echo [OK] Created Desktop Shortcut on your Desktop.
echo.

:CHECK_SERVER
:: 2. Check if Gateway Server (Python) on port 8085 is running
netstat -ano | findstr :8085 >nul
if %errorlevel% equ 0 goto :LAUNCH_BROWSER

echo [INFO] Starting Gateway Server (Python)...
start /B python server.py
echo [INFO] Waiting for server to start...
ping -n 4 127.0.0.1 >nul

:LAUNCH_BROWSER
:: 3. Open browser
echo [INFO] Opening booking website in your browser...
start "" "http://localhost:8085/booking.html?movie=MV001"

echo.
echo ====================================================================
echo [OK] DONE! You can close this window now.
echo ====================================================================
ping -n 3 127.0.0.1 >nul
exit
