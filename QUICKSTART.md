# SilentMind Quick Start Guide 🚀

Get SilentMind up and running in 5 minutes!

## Prerequisites

- **macOS** (for compilation and server)
- **Windows VM** (for client execution)
- **Python 3.7+**
- **mingw-w64** (for cross-compilation)

## Step 1: Setup (2 minutes)

```bash
# Clone and navigate
git clone https://github.com/yourusername/SilentMind.git
cd SilentMind

# Install dependencies
pip3 install -r requirements.txt

# Configure environment
cp .env.example .env
# Edit .env with your API keys
```

## Step 2: Compile (1 minute)

```bash
# Cross-compile for Windows
./compile_windows.sh

# Verify compilation
ls -la silentmind.exe
```

## Step 3: Run Server (1 minute)

```bash
# Start the server
python3 app.py

# You should see:
# * Running on http://0.0.0.0:8000
```

## Step 4: Run Client (1 minute)

```bash
# Copy silentmind.exe to Windows VM
# Run on Windows VM
silentmind.exe

# Check loot directory
ls -la loot/
```

## Expected Output

### Server Logs
```
== [DECISION API HIT] ==
[LLM DECISION RAW]: {"exfiltrate": ["passwords.txt"], "reasons": {...}}
[+] Exfiltrated file saved to loot/20250909-092732__passwords.txt (14 bytes)
[+] Decrypted file saved to loot/20250909-092732__passwords.recovered.txt
```

### Client Output
```
[+] Scanning C:\Users\...\Documents\
[+] Found 3 files
[+] Exfiltrating passwords.txt
[+] Firefox cookies extracted successfully
```

### Loot Directory
```
loot/
├── 20250909-092732__passwords.txt          # Encrypted
├── 20250909-092732__passwords.recovered.txt # Decrypted
├── 20250909-092732__firefox_cookies.txt    # Encrypted
└── 20250909-092732__firefox_cookies.recovered.txt # Decrypted
```

## Troubleshooting

### Compilation Issues
```bash
# Install mingw-w64
brew install mingw-w64

# Check SQLite files
ls -la sqlite3.*
```

### Server Issues
```bash
# Check Python dependencies
pip3 install -r requirements.txt

# Check .env file
cat .env
```

### Client Issues
```bash
# Check server IP in main.c line 165
# Ensure Windows VM can reach server
ping 192.168.1.12
```

## Demo Mode

```bash
# Run the demonstration script
./demo.sh

# This will:
# - Create demo files
# - Show project structure
# - Explain workflow
# - Display sample output
```

## Next Steps

1. **Customize** - Modify server IP, add more file types
2. **Enhance** - Add Chrome cookies, screenshots, keylogger
3. **Deploy** - Use in red team exercises
4. **Contribute** - Submit improvements and features

## Security Note

⚠️ **Use only for authorized testing and educational purposes!**

---

**Need help?** Check the full [README.md](README.md) or [Architecture](ARCHITECTURE.md) documentation.
