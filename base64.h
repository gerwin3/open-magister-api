#ifndef BASE64_H
#define BASE64_H

#include "stream.h"

void base64_encode (stream_t* sin, stream_t* sout);
void base64_decode (stream_t* sin, stream_t* sout);

#endif