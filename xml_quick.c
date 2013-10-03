#include "xml_quick.h"

int xml__read_tag (stream_t* s, char* out)
{
	while (1)
	{
		int i = 0;
		char tag[XML_MAX_TAG];

		memset (tag, 0, XML_MAX_TAG);

		/* wait for any < to show up */
		while (*s_glance (s) != (uint8_t) '<') {
			s_seekg (s, +1);
		}

		s_read_until (s, '>', (uint8_t*) tag, XML_MAX_TAG);

		/* if we've accidentally found a closing tag
		 * we'll just continue the search. */
		s_seekg (s, -1);
		if (s_read_byte (s) == (uint8_t) '/') {
			continue;
		}

		/* read all tag contents */
		s_seekg (s, +1);
		s_read_until (s, '<', (uint8_t*) out, XML_MAX_CONTENT);

		/* read past the end of the tag */
		while (s_read_byte (s) != (uint8_t) '>') {}

		break;
	}

	return 0;
}