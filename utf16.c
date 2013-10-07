#include "utf16.h"

/*
 *	converts utf16 bytes into ascii (7-bit)
 *	characters; removes any non-ascii chars.
 */
int utf16_2_ascii (stream_t* sin, stream_t* sout)
{
	while (!s_eof (sin))
	{
		uint8_t b = 0;
		if ((b = s_read_byte (sin)) <= 128) {
			s_write_byte (sout, b);
		}
	}
}

/*
 *	converts ascii bytes to utf16 by appending
 *	a zero byte after every 7-bit ascii chars;
 *	removes any non-ascii chars.
 */
int ascii_2_utf16 (stream_t* sin, stream_t* sout)
{
	while (!s_eof (sin))
	{
		uint8_t b = 0;
		if ((b = s_read_byte (sin)) <= 128) {
			s_write_byte (sout, b);
			s_write_byte (sout, '\0');
		}
	}
}