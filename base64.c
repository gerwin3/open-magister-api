#include "base64.h"

static const char base64__enc[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
	'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '+', '/'
};
 
static const uint8_t base64__dec[128] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,  62,   0,   0,   0,  63,  52,  53,
	54,   55,  56,  57,  58,  59,  60,  61,   0,   0,
	  0,  64,   0,   0,   0,   0,   1,   2,   3,   4,
	  5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
	 15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
	 25,   0,   0,   0,   0,   0,   0,  26,  27,  28,
	 29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
	 39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
	 49,  50,  51,   0,   0,   0,   0,   0
};

void base64_encode(stream_t* sin, stream_t* sout)
{
	int i = 0;
	int odd = 0;

	while ( ( sin->len - s_tellg (sin)) >= 3)
	{
		uint8_t in[] = { s_read_byte (sin),
				 s_read_byte (sin),
				 s_read_byte (sin) };

		/* split the 3 bytes into 4 6-bit pieces and
		 * use the lookup table to find a matching
		 * character to write to out */
		s_write_byte (sout, base64__enc[(in[0] & 0xfc) >> 2]);
		s_write_byte (sout, base64__enc[( (in[0] & 0x03) << 4) | ( (in[1] & 0xf0) >> 4)]);
		s_write_byte (sout, base64__enc[( (in[1] & 0x0f) << 2) | ( (in[2] & 0xc0) >> 6)]);
		s_write_byte (sout, base64__enc[(in[2] & 0x3f)]);
	}

	odd = ( sin->len - s_tellg (sin));

	if (odd == 1)
	{
		/* if there is 1 byte left we put it into 12-bit
		 * block and read them as two pieces */

		uint8_t in[] = { s_read_byte (sin) };

		s_write_byte (sout, base64__enc[(in[0] & 0xfc) >> 2]);
		s_write_byte (sout, base64__enc[( (in[0] & 0x03) << 4)]);

		/* align to 4 base64 characters! */
		s_write_byte (sout, '=');
		s_write_byte (sout, '=');
	}
	else if (odd == 2)
	{
		/* if there are 2 bytes left we put them
		 * into one 18-bit block and read 3 pieces */

		uint8_t in[] = { s_read_byte (sin),
				 s_read_byte (sin) };

		s_write_byte (sout, base64__enc[(in[0] & 0xfc) >> 2]);
		s_write_byte (sout, base64__enc[( (in[0] & 0x03) << 4) | ( (in[1] & 0xf0) >> 4)]);
		s_write_byte (sout, base64__enc[( (in[1] & 0x0f) << 2)]);

		/* align to 4 base64 characters! */
		s_write_byte (sout, '=');
	}
}

void base64_decode(stream_t* sin, stream_t* sout)
{
	while (!s_eof (sin))
	{
		int odd = 0;
		char in[] = { s_read_byte (sin),
			      s_read_byte (sin),
			      s_read_byte (sin),
			      s_read_byte (sin) };

		/* determine the number of odd (left over)
		 * bytes in the original data */
		if (in[3] == '=') {
			odd = 2;
			if (in[2] == '=') {
				odd = 1;
			}
		}

		switch (odd)
		{
			case 0:
			{
				/* reacquire the 3 bytes from 4 given 6-bit
				 * pieces; write to out */
				s_write_byte (sout, (uint8_t) ( (base64__dec[in[0]] << 2) & 0xff |
								(base64__dec[in[1]] >> 4) & 0xff ) );
				s_write_byte (sout, (uint8_t) ( (base64__dec[in[1]] << 4) & 0xff |
								(base64__dec[in[2]] >> 2) & 0xff ) );
				s_write_byte (sout, (uint8_t) ( (base64__dec[in[2]] << 6) & 0xff |
								base64__dec[in[3]] ) );		
			}
			break;
			case 1:
			{
				/* ... same as above; take into account the
				 * number of odd bytes */
				s_write_byte (sout, (uint8_t) ( (base64__dec[in[0]] << 2) & 0xff |
								(base64__dec[in[1]] >> 4) & 0xff ) );
			}
			break;
			case 2:
			{
				/* ... same as above; take into account the
				 * number of odd bytes */
				s_write_byte (sout, (uint8_t) ( (base64__dec[in[0]] << 2) & 0xff |
								(base64__dec[in[1]] >> 4) & 0xff ) );
				s_write_byte (sout, (uint8_t) ( (base64__dec[in[1]] << 4) & 0xff |
								(base64__dec[in[2]] >> 2) & 0xff ) );
			}
			break;
		}
	}
}
