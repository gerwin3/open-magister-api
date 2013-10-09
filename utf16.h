#ifndef UTF16_H
#define UTF16_H

#include "stream.h"

void utf16_to_ascii (stream_t* sin, stream_t* sout);
void ascii_to_utf16 (stream_t* sin, stream_t* sout);

#endif