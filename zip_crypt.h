#ifndef ZIP_CRYPT
#define ZIP_CRYPT

#include <stdlib.h>
#include <stdint.h>

#include "stream.h"

#define ZIP_CRYPT_HDR_LEN 12

int zip_decrypt (strm_t* sin, strm_t* sout, const char* password, uint32_t seed);
int zip_encrypt (strm_t* sin, strm_t* sout, const char* password, uint32_t seed);

#endif