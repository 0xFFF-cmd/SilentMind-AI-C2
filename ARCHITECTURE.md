# SilentMind Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        SilentMind System                       │
└─────────────────────────────────────────────────────────────────┘
```

## Component Architecture

```
┌─────────────────┐    HTTP POST    ┌─────────────────┐
│   Windows VM    │ ──────────────► │   macOS Server  │
│                 │                 │                 │
│  silentmind.exe │                 │   Python Flask  │
│                 │                 │                 │
│  • File Scanner │                 │  • LLM Decision │
│  • Cookie Extr. │                 │  • Data Storage │
│  • XOR Encrypt  │                 │  • Auto Decrypt │
└─────────────────┘                 └─────────────────┘
```

## Detailed Workflow

```
1. CLIENT STARTUP
   ┌─────────────────┐
   │  silentmind.exe │
   │                 │
   │  • Scan Docs    │
   │  • Get file list│
   └─────────────────┘
            │
            ▼
2. DECISION REQUEST
   ┌─────────────────┐    POST /api/decision    ┌─────────────────┐
   │   Windows VM    │ ──────────────────────► │   macOS Server  │
   │                 │                         │                 │
   │  File List:     │                         │  LLM Analysis:  │
   │  - passwords.txt│                         │  - AI Decision  │
   │  - config.conf  │                         │  - File Ranking │
   │  - readme.txt   │                         │                 │
   └─────────────────┘                         └─────────────────┘
            │                                           │
            │                                           ▼
            │                                   3. LLM RESPONSE
            │                                   ┌─────────────────┐
            │                                   │  Decision JSON: │
            │                                   │  {              │
            │                                   │    "exfiltrate":│
            │                                   │    ["passwords. │
            │                                   │     txt"]       │
            │                                   │  }              │
            │                                   └─────────────────┘
            │                                           │
            │                                           │
            ▼                                           │
4. FILE EXFILTRATION                                    │
   ┌─────────────────┐                                 │
   │   Windows VM    │                                 │
   │                 │                                 │
   │  • XOR Encrypt  │                                 │
   │  • HTTP Upload  │                                 │
   └─────────────────┘                                 │
            │                                           │
            ▼                                           │
   ┌─────────────────┐    POST /api/upload      ┌─────────────────┐
   │   Windows VM    │ ──────────────────────► │   macOS Server  │
   │                 │                         │                 │
   │  Encrypted:     │                         │  • Save Encrypt │
   │  passwords.txt  │                         │  • Auto Decrypt │
   │  (XOR'd)        │                         │  • Save Clear   │
   └─────────────────┘                         └─────────────────┘
            │
            ▼
5. COOKIE EXTRACTION
   ┌─────────────────┐
   │   Windows VM    │
   │                 │
   │  • Find Firefox │
   │  • Parse SQLite │
   │  • Extract Data │
   └─────────────────┘
            │
            ▼
   ┌─────────────────┐    POST /api/upload      ┌─────────────────┐
   │   Windows VM    │ ──────────────────────► │   macOS Server  │
   │                 │                         │                 │
   │  Cookie Data:   │                         │  • Save Encrypt │
   │  - Domain       │                         │  • Auto Decrypt │
   │  - Name/Value   │                         │  • Save Clear   │
   │  - Metadata     │                         │                 │
   └─────────────────┘                         └─────────────────┘
```

## Data Flow

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Raw Data      │───►│  Encryption     │───►│  Transmission   │
│                 │    │                 │    │                 │
│  • Files        │    │  • XOR Cipher   │    │  • HTTP POST    │
│  • Cookies      │    │  • Key: SILENT  │    │  • Base64 Headers│
│  • Metadata     │    │    MIND         │    │  • Binary Body  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                │
                                ▼
                       ┌─────────────────┐    ┌─────────────────┐
                       │   Server        │───►│   Storage       │
                       │                 │    │                 │
                       │  • Auto Decrypt │    │  • Encrypted    │
                       │  • Parse Data   │    │  • Decrypted    │
                       │  • Save Both    │    │  • Organized    │
                       └─────────────────┘    └─────────────────┘
```

## Security Layers

```
┌─────────────────────────────────────────────────────────────────┐
│                        Security Layers                         │
├─────────────────────────────────────────────────────────────────┤
│  Layer 1: XOR Encryption (Client-side)                         │
│  Layer 2: HTTP Transmission (Network)                          │
│  Layer 3: Server-side Decryption (Server)                      │
│  Layer 4: File System Storage (Persistent)                     │
└─────────────────────────────────────────────────────────────────┘
```

## File Structure

```
SilentMind/
├── client/                    # Windows Client
│   ├── main.c                # Entry point & orchestration
│   ├── encrypt.c/.h          # XOR encryption functions
│   ├── exfil_http.c          # HTTP communication
│   └── firefox_cookies.c/.h  # Cookie extraction
├── server/                   # Python Server
│   ├── llm.py               # AI decision making
│   └── routes/               # API endpoints
│       ├── decision.py      # File analysis endpoint
│       └── upload.py        # Data reception endpoint
├── loot/                    # Exfiltrated Data
│   ├── *.txt               # Encrypted files
│   └── *.recovered.txt     # Decrypted files
└── tools/                   # Utilities
    └── xor_decrypt.py      # Manual decryption
```

## API Endpoints

```
┌─────────────────┐    POST /api/decision    ┌─────────────────┐
│   Client        │ ──────────────────────► │   Server        │
│                 │                         │                 │
│  Request:       │                         │  Response:      │
│  {              │                         │  {              │
│    "fingerprint":│                         │    "exfiltrate":│
│    {...},       │                         │    [...],       │
│    "files": [...]│                         │    "reasons":   │
│  }              │                         │    {...}        │
│                 │                         │  }              │
└─────────────────┘                         └─────────────────┘

┌─────────────────┐    POST /api/upload      ┌─────────────────┐
│   Client        │ ──────────────────────► │   Server        │
│                 │                         │                 │
│  Headers:       │                         │  Process:       │
│  • Content-Type │                         │  • Save Encrypt │
│  • X-Filename   │                         │  • XOR Decrypt  │
│                 │                         │  • Save Clear   │
│  Body:          │                         │                 │
│  • XOR Data     │                         │                 │
└─────────────────┘                         └─────────────────┘
```

## Error Handling

```
┌─────────────────┐
│   Error Types   │
├─────────────────┤
│  • File Access  │
│  • Network      │
│  • Database     │
│  • Encryption   │
│  • LLM API      │
└─────────────────┘
            │
            ▼
┌─────────────────┐
│  Error Handling │
├─────────────────┤
│  • Graceful     │
│  • Silent       │
│  • Logging      │
│  • Recovery     │
└─────────────────┘
```

## Performance Considerations

```
┌─────────────────┐
│   Performance   │
├─────────────────┤
│  • File Size    │
│  • Network      │
│  • Memory       │
│  • CPU Usage    │
│  • Disk I/O     │
└─────────────────┘
            │
            ▼
┌─────────────────┐
│  Optimizations  │
├─────────────────┤
│  • Chunked      │
│  • Streaming    │
│  • Compression  │
│  • Caching      │
└─────────────────┘
```
