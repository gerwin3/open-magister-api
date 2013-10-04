#include "ma.h"

#include <curl/curl.h>
/*#include <libxml/parser.h>
#include <libxml/tree.h>		TODO: Reinstate, we're not ready yet. */
#include <zlib.h>

#include "hex.h"
#include "zip.h"
#include "zip_crypt.h"

int ma__receive_callback (void* buf, size_t size, size_t len, void* userp)
{
	s_write ( (stream_t*) userp, (uint8_t*) buf, size * len);

	return len;
}

int ma__header_callback (void* buf, size_t size, size_t len, void* userp)
{
	stream_t* shdr = (stream_t*) userp;

	int i = 0;
	for (i = 0; i < MAX_NHDRS; i++)
	{
		char* sstr =
			(char*) s_glance (shdr);

		/* write the header to the stream if it is actually the
		 * <X from instrm> header */
		if (memcmp (sstr, buf, strlen (sstr)) == 0)
		{
			s_seekp (shdr, S_SEEK_BEGIN);
			s_write (shdr, (uint8_t*) buf, (len * size));

			break;
		}
	}

	return len;
}

int ma__decode_request (stream_t* sin, stream_t* sout)
{
	/* init */

	int r = 0;
	int size = 0;
	stream_t s1, s2, s3, s4;

	z_stream zs;
	struct zip_file_info zfile;

	/* decode */

	/*
	 *	L1: SOAP layer
	 *	 -> simply skip the header
	 */
	s_seekg (sin, + strlen (MA_SOAP_PREFIX));

	/*
	 *	L2: hexdec layer
	 *	 -> convert to binary with hex2bin ()
	 */
	size = (sin->len - (strlen (MA_SOAP_PREFIX) + strlen (MA_SOAP_POSTFIX))) / 2;
	s1 = s_create (size);
	hex2bin (sin, &s1);

	/*
	 *	L3: Zip container
	 *	 -> extract the 'content' file from the zip
	 */
	s2 = s_create (size);
	zip_file_read (&s1, "content", &zfile, &s2);

	/*
	 *	L4: PK Zip encryption
	 *	 -> decrypt with MA_ZIP_PASSWORD
	 */
	s3 = s_create (zfile.uncomp_size);
	if (zip_decrypt (&s2, &s3, MA_ZIP_PASSWORD, zfile.mod_time) != 0)
	{
		/* Log Warning */
	}

	/*
	 *	L5: DEFLATE compressed bytes
	 *	 -> uncompress them
	 */
	*sout = s_create (zfile.uncomp_size);

	zs.zalloc = Z_NULL;
	zs.zfree = Z_NULL;
	zs.opaque = Z_NULL;
	zs.avail_in = (uInt) s3.len;
	zs.next_in = (Bytef*) s3.buf;
	zs.avail_out = (uInt) sout->len;
	zs.next_out = (Bytef*) sout->buf;

	inflateInit2 (&zs, -13);
	inflate (&zs, Z_NO_FLUSH);
	inflateEnd (&zs);
	{
		char* debugtodoremovex = (char*) sout->buf;
		int i = 0;
	}
	

	/*
	 *	L6: XML
	 *	 -> parse the xml data
	 */
	{/*
		xmlDocPtr doc =
			xmlParseMemory ( (const char*) s_glance (&s4), s4.len);

		xmlNodePtr cur
			= xmlDocGetRootElement (doc);

		cur = cur->xmlChildrenNode;
		while (cur != NULL)
		{
			if ( (!xmlStrcmp (cur->name, (const xmlChar *) "XXX"))){
				
			}
			cur = cur->next;
		}*/
	}

	s_seekg (sin, + strlen (MA_SOAP_POSTFIX));

	/* deinit */

	s_free (&s1);
	s_free (&s2);
	s_free (&s3);
	s_free (&s4);
}

int ma__do_request (struct ma_medius* m, const char* service, stream_t* req)
{
	/* init */

	int ret = 0;

	CURL* curl = curl_easy_init();
	char url[MAX_PATH];

	stream_t isresp =
		s_create_from_buf (m->buf, MAX_CONTENT);

	stream_t osresp =
		s_create_from_buf (m->buf, MAX_CONTENT);

	memset (m->buf, 0, MAX_CONTENT);
	memset (url, 0, MAX_PATH);

	/* prepare for request */

	sprintf (url, "https://%s/%s/WCFServices/%s",
			 m->url_base,
			 m->url_v,
			 service);

	curl_easy_setopt (curl, CURLOPT_URL, url);
	curl_easy_setopt (curl, CURLOPT_URL, url);
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);

	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, ma__receive_callback);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, &isresp);

	/* request ! */

	if (curl_easy_perform (curl) == CURLE_OK)
	{
		if (ma__decode_request (&isresp, &osresp) == MA_OK)
		{
			m->len = s_tellp (&osresp);
			ret = MA_OK;
		}
		else {
			ret = MA_EMALFORMED;
		}
	}
	else {
		ret = MA_ECONNECTION;
	}

	s_free (&isresp);
	s_free (&osresp);

	return ret;
}

int ma_medius_init (struct ma_medius* m, const char* name)
{
	/* init */

	CURL* curl = curl_easy_init();
	int ret = 0;

	int i = 0;
	
	char url[MAX_URL];

	stream_t resp_body;

	char hdr[MAX_URL];
	stream_t resp_hdr =
		s_create_from_buf ( (uint8_t*) hdr, MAX_URL);

	memset (url, 0, MAX_URL);

	m->buf = (uint8_t*) malloc (MAX_CONTENT);
	resp_body =
		s_create_from_buf (m->buf, MAX_CONTENT);

	memset (hdr, 0, MAX_URL);

	/* prepare for request */

	sprintf (url, "https://%s/", name);
	strcat (hdr, "Location");

	curl_easy_setopt (curl, CURLOPT_URL, url);
	curl_easy_setopt (curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);

	curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, ma__header_callback);
	curl_easy_setopt (curl, CURLOPT_WRITEHEADER, &resp_hdr);

	/* request ! */

	if (curl_easy_perform (curl) == CURLE_OK)
	{
		if (s_tellp (&resp_hdr) > 0)
		{
			memset (m->url_base, 0, MAX_URL);
			memset (m->url_v, 0, MAX_URL);

			strcpy (m->url_base, name);

			s_seekg (&resp_hdr, + (strlen ("Location: /")));
			s_read_until (&resp_hdr, '/', (uint8_t*) m->url_v, MAX_URL);
		}
		else {
			ret = MA_EMALFORMED;
		}
	}
	else {
		ret = MA_ECONNECTION;
	}

	s_free (&resp_body);
	s_free (&resp_hdr);

	return ret;
}

void ma_medius_delete (struct ma_medius* m)
{
	free (m->buf);
}