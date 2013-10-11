#include "zip_crypt.h"

#include <time.h>
#include <string.h>

#include "crc32.h"

void zip_crypt__develop_keys (uint32_t keys[3], uint8_t x)
{
	keys[0] = CRC32 (keys[0], x);
	keys[1] += keys[0] & 0xff;
	keys[1] = keys[1] * 134775813 + 1;
	keys[2] = CRC32 (keys[2], keys[1] >> 24);
}

uint8_t zip_crypt__special_byte (uint32_t keys[3])
{
	uint16_t y =
		(uint16_t) keys[2] | 2;
	
	return (uint8_t)
		( ( (uint16_t) (y * (y ^ 1)) >> 8) & 0xff);
}

int zip_encrypt (stream_t* sin, stream_t* sout, const char* password, uint32_t seed)
{
	uint32_t keys[3] = { 305419896,
			     591751049,
			     878082192 };

	int i = 0;
	uint32_t rnd = (clock () ^ (seed << 5));

	/* develops the keys with the password */
	for (i = 0; i < strlen (password); i++) {
		zip_crypt__develop_keys (keys, password[i]);
	}

	/* encrypt and write (& develop keys!) to sout */
	for (i = 0; !s_eof (sin); i++)
	{
		uint8_t x = 0;

		if (i >= ZIP_CRYPT_HDR_LEN)
		{
			x = s_read_byte (sin);
		}
		else if (i < (ZIP_CRYPT_HDR_LEN - 2))
		{
			/* we're in the first 10 bytes of the header;
			 * we must write crap */
			x = (uint8_t)
				( ( ( (rnd >> (i * 3)) & 0xff) + (i * 23) ) ) % 256;
		}
		else if (i == (ZIP_CRYPT_HDR_LEN - 2))
		{
			/* first validation byte, some implementations
			 * don't check this (including this one) */
			x = (seed & 0xff);
		}
		else if (i == (ZIP_CRYPT_HDR_LEN - 1))
		{
			/* the validation byte, can be used to validate
			 * decoded data */
			x = ((seed >> 8) & 0xff);
		}

		/* xor-encrypt the byte with another special
		 * byte; then develop the keys */
		s_write_byte (sout, zip_crypt__special_byte (keys) ^ x);

		zip_crypt__develop_keys (keys, x);
	}

	return 0;
}

int zip_decrypt (stream_t* sin, stream_t* sout, const char* password, uint32_t seed)
{
	int r = 0;

	uint32_t keys[3] = { 305419896,
			     591751049,
			     878082192 };

	int i = 0;

	/* develops the keys with the password */
	for (i = 0; i < strlen (password); i++) {
		zip_crypt__develop_keys (keys, password[i]);
	}

	for (i = 0; !s_eof (sin); i++)
	{
		/* decrypt read byte with the special byte => x */
		uint8_t x =
			s_read_byte (sin) ^ zip_crypt__special_byte (keys);

		zip_crypt__develop_keys (keys, x);

		/* this byte can enables us to check whether the pass-
		 * word is correct... */
		if (i == ZIP_CRYPT_HDR_LEN - 1)
		{
			r = ((x == (seed >> 8) & 0xff)
				? ZIP_CRYPT_OK
				: ZIP_CRYPT_BAD_PASSWORD);
		}

		/* we don't need to decrypt and store the header, so
		 * only write to the output stream after 12 bytes */
		if (i >= ZIP_CRYPT_HDR_LEN) {
			s_write_byte (sout, x);
		}
	}

	return r;
}

