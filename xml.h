#ifndef XML_H
#define XML_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "stream.h"
#include "list.h"

#define XML_OK				0
#define XML_EMALFORMED		-1
#define XML_EINVAL			-2

#define XML_MAX_TAG			64
#define XML_MAX_CONTENT		1000000

struct xml_tag
{
	char	name[XML_MAX_TAG];
	char*	content;			/* a pointer to some location inside
								 * the provided stream */
	int		depth;
};

extern int xml_quick_parse (stream_t* sin, llist_t* out);
extern int xml_quick_write (llist_t* in, stream_t* sout);

#endif