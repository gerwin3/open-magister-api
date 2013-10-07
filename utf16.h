#ifndef UTF16_H
#define UTF16_H

#include "stream.h"

extern int utf16_2_ascii (stream_t* sin, stream_t* sout);
extern int ascii_2_utf16 (stream_t* sin, stream_t* sout);

#endif