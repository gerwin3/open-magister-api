#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <Windows.h> /* don't worry, just for testing! */

#include "stream.h"
#include "ma.h"

#include "zip_crypt.h"

/*	MODULE		  |	STATUS		  |	LIB			  |	
 *	-----------------------------------------------
 *	hex <-> bin		Done			hex.h / My
 *	zip headers		>Untested<Done	zip.h / My
 *	zip crypto		Done			zypt.h / Xceed
 *	XML				Poss. Obsolete	?
 *	SOAP			Done			? / My
 *	HTTPS			Done			curl (.h)
 *	DEFLATE			Done (Unused)	zlib (.h)
 */

int easy_fread (char* path, stream_t* s)
{
	int r = -1;
	FILE* f = fopen (path, "rb");

	if (f > 0)
	{
		uint8_t* fcontents = NULL;
		int fsz = 0;

		fseek (f, 0, SEEK_END);
		fsz = ftell (f);
		rewind (f);

		fcontents = (uint8_t*) malloc (fsz);

		if (fread ( (char*) fcontents, 1, fsz, f) == fsz) {
			r = 0;
		} else {
			r = -1;
		}

		s_write (s, fcontents, fsz);

		free (fcontents);
		fclose (f);
	}
	else {
		r = -1;
	}

	return r;
}

int easy_fwrite (char* path, stream_t* s)
{
	int r = -1;
	FILE* f = fopen (path, "wb");

	if (f > 0)
	{
		if (fwrite ( (char*) s_glance (s), 1, s->len, f) == s->len) {
			/* pretend we read it */
			s_seekg (s, S_SEEK_END);
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
	int ret;
	stream_t s1 = s_create ();
	stream_t s2 = s_create ();
	stream_t s3 = s_create ();

	s_write (&s1, (uint8_t*) "To beer or not to beer. Beer.",
					 strlen ("To beer or not to beer. Beer."));

	if (zip_encrypt (&s1, &s2, "SeCrEt!", 1337) != ZIP_CRYPT_OK) {
		return -1;
	}

	ret = zip_decrypt (&s2, &s3, "SeCrEt!", 1337);

	ret = (ret == 0);

// 	
// 	char refpath[MAX_PATH] = "J:\\Programming\\staging_workspace\\magister\\comm\\all-resp\\resp";
// 
// 	int i = 0;
// 	for (i = 1; i <= 17; i++)
// 	{
// 		char path[MAX_PATH];
// 		char suffix[] = "\0\0";
// 
// 		stream_t sencoded = s_create ();
// 		stream_t sdecoded = s_create ();
// 
// 		/* construct path */
// 		strcpy (path, refpath);
// 		itoa (i, suffix, 10);
// 		strcat (path, suffix);
// 
// 		/* read & decode */
// 		easy_fread (path, &sencoded);
// 		ma__decode_request (&sencoded, &sdecoded);
// 		
// 		/* write to .out path */
// 		strcat (path, ".out");
// 		easy_fwrite (path, &sdecoded);	
// 	}
}