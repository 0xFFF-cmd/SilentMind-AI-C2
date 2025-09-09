import sys, os

KEY = b"SILENTMIND"

def xor_bytes(data: bytearray, key: bytes):
    k = len(key)
    for i in range(len(data)):
        data[i] ^= key[i % k]

def main():
    if len(sys.argv) < 2:
        print("Usage: python xor_decrypt.py <loot_file> [--out recovered.bin] | python tools/xor_decrypt.py loot/20250903-123532__RFP\ -\ Red\ Teaming\ \ 21\ July\ .pdf")
        sys.exit(1)

    src = sys.argv[1]
    out = None
    if len(sys.argv) >= 4 and sys.argv[2] == "--out":
        out = sys.argv[3]
    else:
        root, ext = os.path.splitext(src)
        out = root + ".recovered" + (ext if ext else "")

    data = bytearray(open(src, "rb").read())
    xor_bytes(data, KEY)
    open(out, "wb").write(data)
    print(f"[+] Wrote {out}")

if __name__ == "__main__":
    main()
