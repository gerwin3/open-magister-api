#include "stream.h"

#include <limits.h>

/*
 *	Increase the g offset, check for eof, return the
 *	value (if != yet eof).
 */
#define S_READ_INTX(Type) \
	\
	if ( (s->g + sizeof (Type) > s->len)) { \
		return (Type) 0; \
	} \
	\
	s->g += sizeof (Type); \
	\
	return * ( (Type*) &s->buf[s->g - sizeof (Type)]);

/*
 *	Increase the p offset, check for eof, possibly
 *	realloc some space, append value x.
 */
#define S_WRITE_INTX(Type) \
	\
	if ( (s->p + sizeof (Type) > s->len_avail)) { \
		s__realloc_to_fit (s, s->p + sizeof (Type)); \
	} \
	\
	if ( (s->p + sizeof (Type) > s->len)) { \
		s->len = s->p + sizeof (Type); \
	} \
	\
	*( (Type*) &s->buf[s->p]) = x; \
	s->p += sizeof (Type);

void s__realloc_to_fit (stream_t* s, int minlen)
{
	/* buffer will be reallocated, the new size is
	 * twice the size of the minimum to make sure
	 * we don't keep reallocating */
	s->buf = (uint8_t*)
		realloc (s->buf, (minlen * 2));

	s->len_avail = (minlen * 2);
}

stream_t s_create ()
{
	stream_t s;
	s.buf = (uint8_t*) malloc (S_INIT_LEN);
	s.buf_own = 1;
	s.len_avail = S_INIT_LEN;
	s.len = 0;
	s.g = s.p = 0;

	return s;
}

stream_t s_create_from_buf (uint8_t* buf, int len)
{
	stream_t s;
	s.buf = buf;
	s.buf_own = 0;
	s.len_avail = len;
	s.len = len;
	s.g = s.p = 0;

	return s;
}

void s_free (stream_t* s)
{
	if (s->buf_own) {
		free (s->buf);
	}
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

void s_rewind (stream_t* s)
{
	s->g = 0;
	s->p = 0;
}

int s_eof (stream_t* s)
{
	return (s->g >= s->len);
}

int s_read (stream_t* s, uint8_t* b, int len)
{
	int i = 0;
	for (i = 0; (i < len && !s_eof (s)); i++)
	{
		/* only write if we have a valid
		 * output buffer */
		if (b != NULL) {
			b[i] = s->buf[s->g];
		}

		s->g++; /* move get ptr */
	}

	return i;
}

void s_write (stream_t* s, uint8_t* b, int len)
{
	int i = 0;

	/* make sure we can fit this in */
	if (s->p + len > s->len_avail) {
		s__realloc_to_fit (s, s->p + len);
	}

	/* adjust len if necessary */
	if (s->p + len > s->len) {
		s->len = s->p + len;
	}

	/* copy over the data if we have
	 * a valid output buffer */
	for (i = 0; i < len; i++)
	{
		if (b != NULL) {
			s->buf[s->p] = b[i];
		}

		s->p++; /* move put ptr */
	}
}

int s_read_until (stream_t* s, char delim, uint8_t* b, int maxlen)
{
	int i = 0;
	for (i = 0; (i < ( (maxlen > 0) ? maxlen : INT_MAX) && !s_eof (s)); i++)
	{
		/* make sure we stop reading before
		 * reaching the delim */
		if (s_peek (s) == (uint8_t) delim) {
			break;
		}

		if (b != NULL) {
			/* only write to valid buf */
			b[i] = s->buf[s->g];
		}

		s->g++; /* move get ptr */
	}

	return i;
}

uint8_t* s_glance(stream_t* s)
{
	/* get a offset pointer to the data;
	 * only for advanced usage */
	return (s->buf + s->g);
}

uint8_t s_peek(stream_t* s)
{
	return (s->buf[s->g]);
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