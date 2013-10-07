#ifndef STREAM_H
#define STREAM_H

#include <stdlib.h>
#include <stdint.h>

#define S_INIT_LEN		256

#define S_SEEK_BEGIN	INT_MIN
#define S_SEEK_END		INT_MAX

typedef struct
{
	uint8_t*	buf;	/* gets resized when more space is needed, size 
						 * always equals .len */
	int			buf_own;
	int			len_avail;
	int			len;
	int			g;
	int			p;
}
stream_t;

stream_t s_create ();
stream_t s_create_from_buf (uint8_t* buf, int len);

void s_free (stream_t* s);

void s_seekg (stream_t* s, int relpos);
void s_seekp (stream_t* s, int relpos);
int s_tellg (stream_t* s);
int s_tellp (stream_t* s);

void s_rewind (stream_t* s);

int s_eof (stream_t* s);

int s_read (stream_t* s, uint8_t* b, int len);
void s_write (stream_t* s, uint8_t* b, int len);

int s_read_until (stream_t* s, char delim, uint8_t* b, int maxlen);

uint8_t* s_glance (stream_t* s);
uint8_t s_peek (stream_t* s);

uint8_t s_read_byte (stream_t* s);
void s_write_byte (stream_t* s, uint8_t x);
uint16_t s_read_int16 (stream_t* s);
void s_write_int16 (stream_t* s, uint16_t x);
uint32_t s_read_int32 (stream_t* s);
void s_write_int32 (stream_t* s, uint32_t x);
uint64_t s_read_int64 (stream_t* s);
void s_write_int64 (stream_t* s, uint64_t x);

#endif