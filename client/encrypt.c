// encrypt.c
#include "encrypt.h"

void xor_encrypt(unsigned char *data, int data_len, const char *key, int key_len) {
    for (int i = 0; i < data_len; i++) {
        data[i] ^= key[i % key_len];
    }
}