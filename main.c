#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/*#include <curl/curl.h>*/

#include "stream.h"
#include "ma.h"

/*	MODULE		  |	STATUS		  |	LIB			  |	
 *	-----------------------------------------------
 *	hex <-> bin		Done			hex.h / My
 *	zip headers		Untested		zip.h / My
 *	zip crypto		Untested		zypt.h / Xceed
 *	XML				Progressing		?
 *	SOAP			Done			? / My
 *	HTTPS			Done			curl (.h)
 *	DEFLATE			Untested		zlib (.h)
 */

int easy_fread (char* path, strm_t* s)
{
	int r = -1;
	FILE* f = fopen (path, "rb");

	if (f > 0)
	{
		int fsz = 0;

		fseek (f, 0, SEEK_END);
		fsz = ftell (f);
		rewind (f);

		*s = s_create (fsz);

		if (fread (s->buf, 1, fsz, f) == fsz) {
			r = 0;
		} else {
			r = -1;
		}

		fclose (f);
	}
	else {
		r = -1;
	}

	return r;
}

int main (int argc, char* argv[])
{
	strm_t s;

	easy_fread ("J:\\Programming\\staging_workspace\\magister\\comm\\all-req\\req1", &s);

	ma__decode_request (&s, &s /*BOGUS*/);

// 	struct ma_medius m;
// 	int r;
// 
// 	curl_global_init (CURL_GLOBAL_ALL);
// 	
// 	r = ma_medius_init (&m, "sga.swp.nl");
// 
// 	/*
// 		test:
// 			read all files in {f} and
// 				for each f in {f}
// 					ma__decode_request ( f as stream,
// 										 stdout as stream )
// 
// 	/*
// 		.... do stuff ....
// 
// 		...
// 	 */
// 
// 
// 
// 	ma_medius_delete (&m);
// 
// 	curl_global_cleanup ();
}