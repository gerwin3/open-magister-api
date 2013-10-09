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

int xml_find_tag (stream_t* sin, const char* name, struct xml_tag* tag);
int xml_write_tag (llist_t* in, stream_t* sout, int sformat);

#endif