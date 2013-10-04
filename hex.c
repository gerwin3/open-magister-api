#include "hex.h"

#include <stdio.h>
#include <stdint.h>

/*
 *	Formats binary data as a hexdecimal stream, each byte is
 *	converted into two hexdecimal digits 0-f.
 */
void bin2hex (stream_t* sin, stream_t* sout)
{
	while (!s_eof (sin))
	{
		char hex[3];
		char byte = (char) s_read_byte(sin);

		sprintf (hex, "%02X", byte);

		s_write_byte (sout, hex[0]);
		s_write_byte (sout, hex[1]);
	}
}

/*
 *	Formats a hexdecimal stream into a normal binary stream,
 *	takes 2 digits (0-f) each and writes a byte to stream out.
 */
void hex2bin (stream_t* sin, stream_t* sout)
{
	while (!s_eof(sin))
	{
		char hex[] = { (char) s_read_byte(sin),
					   (char) s_read_byte(sin),
					   '\0' };
		int byte;
		
		sscanf (hex, "%2hhX", &byte);

		s_write_byte (sout, (uint8_t) byte);
	}
}