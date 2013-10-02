#ifndef STREAM_H
#define STREAM_H

#include <stdlib.h>
#include <stdint.h>

#define S_MODE_G 0
#define S_MODE_P 1

#define S_SEEK_BEGIN	INT_MIN
#define S_SEEK_END		INT_MAX

typedef struct
{
	uint8_t* buf;
	int len;
	int g;			/* get offset, use strm_seekg/strm_tellg */
	int p;			/* put offset, use strm_seekp/strm_tellp */
	int own;
} strm_t;

strm_t s_create (int len);
strm_t s_create_from_buf (uint8_t* buf, int len);

void s_delete (strm_t* s);

void s_seekg (strm_t* s, int relpos);
void s_seekp (strm_t* s, int relpos);
int s_tellg (strm_t* s);
int s_tellp (strm_t* s);

void s_reset (strm_t* s);

int s_eof (strm_t* s, int mode);

void s_read (strm_t* s, uint8_t* b, int b_len);
void s_write (strm_t* s, uint8_t* b, int b_len);

void s_read_until (strm_t* s, char delim, uint8_t* b, int b_len);

uint8_t* s_glance (strm_t* s);

uint8_t s_read_byte (strm_t* s);
void s_write_byte (strm_t* s, uint8_t x);
uint16_t s_read_int16 (strm_t* s);
void s_write_int16 (strm_t* s, uint16_t x);
uint32_t s_read_int32 (strm_t* s);
void s_write_int32 (strm_t* s, uint32_t x);
uint64_t s_read_int64 (strm_t* s);
void s_write_int64 (strm_t* s, uint64_t x);

#endif