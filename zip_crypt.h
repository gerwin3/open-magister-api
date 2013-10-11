#ifndef ZIP_CRYPT
#define ZIP_CRYPT

#include <stdlib.h>
#include <stdint.h>

#include "stream.h"

#define ZIP_CRYPT_OK		0
#define ZIP_CRYPT_BAD_PASSWORD	-1

#define ZIP_CRYPT_HDR_LEN	12

int zip_decrypt (stream_t* sin, stream_t* sout, const char* password, uint32_t seed);
int zip_encrypt (stream_t* sin, stream_t* sout, const char* password, uint32_t seed);

#endif