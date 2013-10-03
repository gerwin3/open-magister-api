#include "stream.h"

/*
 *	increase the g offset, check for eof, return the
 *	value (if != yet eof) with a adjusted index
 */
#define S_READ_INTX(Type) \
	if (s__would_eof (s, sizeof(Type), S_MODE_G)) return 0; \
	s->g += sizeof(Type); \
	return *((Type*)&s->buf[s->g - sizeof(Type)]);

/*
 *	increase the p offset, check for eof, return the
 *	value (if != yet eof) with a adjusted index
 */
#define S_WRITE_INTX(Type) \
	if (s__would_eof (s, sizeof(Type), S_MODE_P)) return; \
	*((Type*)&s->buf[s->p]) = x; \
	s->p += sizeof(Type);

int s__would_eof (stream_t* s, int w)
{
	return (s->g + w > s->len);
}

stream_t s_create ()
{
	stream_t s;
	s.buf = (uint8_t*) malloc (S_INIT_LEN);
	s.len = S_INIT_LEN;
	s.g = s.p = 0;

	return s;
}

stream_t s_create_from_buf (uint8_t* buf, int len)
{
	stream_t s;
	s.buf = buf;
	s.len = len;
	s.g = s.p = 0;

	return s;
}

void s_delete (stream_t* s)
{
	
}

void s_seekg (stream_t* s, int relpos)
{
	if (relpos == S_SEEK_BEGIN) {
		s->g = 0;
	} else if (relpos == S_SEEK_END) {
		s->g = s->len;
	} else {
		s->g += relpos;
	}
}

void s_seekp (stream_t* s, int relpos)
{
	if (relpos == S_SEEK_BEGIN) {
		s->p = 0;
	} else if (relpos == S_SEEK_END) {
		s->p = s->len;
	} else {
		s->p += relpos;
	}
}

int s_tellg (stream_t* s)
{
	return s->g;
}

int s_tellp (stream_t* s)
{
	return s->p;
}

void s_reset (stream_t* s)
{
	s->g = 0;
	s->p = 0;
}

int s_eof (stream_t* s, int mode)
{
	return (s->g > s->len);
}

void s_read (stream_t* s, uint8_t* b, int b_len)
{
	int i = 0;
	for (i = 0; i < b_len; i++) {
		b[i] = s->buf[i + s->g];
	}
	s->g += b_len;
}

void s_write (stream_t* s, uint8_t* b, int b_len)
{
	int i = 0;
	for (i = 0; i < b_len; i++) {
		s->buf[i + s->p] = b[i];
	}
	s->p += b_len;
}

void s_read_string(stream_t* s, char* str, int str_len, int format)
{
	switch (format)
	{
		/*TODO !!!!*/
	}
}

void s_write_string(stream_t* s, char* str, int str_len, int format)
{
	switch (format)
	{
		/*TODO !!!!*/
	}
}

void s_read_until (stream_t* s, char delim, uint8_t* b, int b_len)
{
	int i = 0;
	for (i = 0; i < b_len; i++)
	{
		if (*s_glance (s) == (uint8_t) delim) {
			break;
		}

		b[i] = s_read_byte (s);
	}
}

uint8_t* s_glance(stream_t* s)
{
	return (s->buf + s->g);
}

uint8_t s_read_byte (stream_t* s)
{
	S_READ_INTX(uint8_t)
}

void s_write_byte (stream_t* s, uint8_t x)
{
	S_WRITE_INTX(uint8_t)
}

uint16_t s_read_int16 (stream_t* s)
{
	S_READ_INTX(uint16_t)
}

void s_write_int16 (stream_t* s, uint16_t x)
{
	S_WRITE_INTX(uint16_t)
}

uint32_t s_read_int32 (stream_t* s)
{
	S_READ_INTX(uint32_t)
}

void s_write_int32 (stream_t* s, uint32_t x)
{
	S_WRITE_INTX(uint32_t)
}

uint64_t s_read_int64 (stream_t* s)
{
	S_READ_INTX(uint64_t)
}

void s_write_int64 (stream_t* s, uint64_t x)
{
	S_WRITE_INTX(uint64_t)
}