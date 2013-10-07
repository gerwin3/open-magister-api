#include "xml.h"

int xml_quick_parse (stream_t* s, llist_t* out)
{
	int depth = 0;
	while (!s_eof (s))
	{
		
	}

	/*
	s_read_until (s, '>', (uint8_t*) tag, XML_MAX_TAG);

	while (1)
	{
		int i = 0;
		char tag[XML_MAX_TAG];

		memset (tag, 0, XML_MAX_TAG);

		/* wait for any < to show up *
		while (*s_glance (s) != (uint8_t) '<') {
			s_seekg (s, +1);
		}

		s_read_until (s, '>', (uint8_t*) tag, XML_MAX_TAG);

		/* if we've accidentally found a closing tag
		 * we'll just continue the search. *
		s_seekg (s, -1);
		if (s_read_byte (s) == (uint8_t) '/') {
			continue;
		}

		/* read all tag contents *
		s_seekg (s, +1);
		s_read_until (s, '<', (uint8_t*) out, XML_MAX_CONTENT);

		/* read past the end of the tag *
		while (s_read_byte (s) != (uint8_t) '>') {}

		break;
	}

	return 0;*/
}