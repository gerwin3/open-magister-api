#include "ma.h"

#include <curl/curl.h>
#include <zlib.h>

#include "list.h"
#include "utf16.h"
#include "hex.h"
#include "zip.h"
#include "zip_crypt.h"

/*
 *	callback - utilized by curl, used by us - to read
 *	response data
 *	 - buf: response data
 *	 - size: byte size
 *	 - len: num byte
 *	 - userp: pointer provided by us; will be converted
 *			  to stream_t* to write output data
 */
int ma__receive_callback (void* buf, size_t size, size_t len, void* userp)
{
	s_write ( (stream_t*) userp,
			  (uint8_t*) buf,
			  (size * len));

	return len;
}

/*
 *	callback - utilized by curl, used by us - to read
 *	response headers
 *	 - buf: header line bytes
 *	 - size: byte size
 *	 - len: num byte
 *	 - userp: pointer provided by us; will be converted
 *			  to llist_*, containing header-var's that
 *			  are to be replaced by their values
 */
int ma__header_callback (void* buf, size_t size, size_t len, void* userp)
{
	llist_t* lhdrs = (llist_t*) userp;
	struct llist_node* nhdr = NULL;

	/* we're trying to find out if this is one of the
	 * headers we're looking for */
	for (nhdr = lhdrs->head;
		 nhdr->next != NULL;
		 nhdr = nhdr->next)
	{
		char* shdr = (char*) nhdr->v;

		if (memcmp (shdr, buf, strlen (shdr)))
		{
			/* we've found a header we need, reset the
			 * input buffer and copy over this line to
			 * it; now it's a output buf */
			memset (shdr, 0, MAX_HDR);
			memcpy (shdr, buf, max ( (size * len), MAX_HDR));
		}
	}

	return len;
}

/*
 *	decodes data from the magister servers
 *	 - sin: input data
 *	 - sout: output data
 */
int ma__decode_request (stream_t* sin, stream_t* sout)
{
	/* init */

	int r = 0;
	stream_t s1, s2, s3;

	struct zip_file_info zfile;

	s1 = s_create ();
	s2 = s_create ();
	s3 = s_create ();

	/* decode */

	{
		/*
		*	L1: SOAP layer
		*	 -> simply skip the header
		*/
		s_seekg (sin, + strlen (MA_SOAP_PREFIX));
	}

	{
		/*
		*	L2: hexdec layer
		*	 -> convert to binary with hex2bin ()
		*/
		hex2bin (sin, &s1);
	}

	{
		/*
		*	L3: Zip container
		*	 -> extract the 'content' file from the zip
		*/
		zip_file_read (&s1, "content", &zfile, &s2);
	}

	{
		/*
		*	L4: PK Zip encryption
		*	 -> decrypt with MA_ZIP_PASSWORD
		*/
		if (zip_decrypt (&s2, &s3, MA_ZIP_PASSWORD, zfile.mod_time) != 0)
		{
			/* Log Warning */
		}
	}

	{
		/*
		*	L5: DEFLATE compressed bytes
		*	 -> uncompress them
		*/
		uint8_t* z_buf_out = (uint8_t*)
			malloc (zfile.uncomp_size);

		z_stream zs;
		zs.zalloc = Z_NULL;
		zs.zfree = Z_NULL;
		zs.opaque = Z_NULL;
		zs.avail_in = (uInt) s3.len;	/* we've gotta hack these out of there ... */
		zs.next_in = (Bytef*) s3.buf;	/* ... to make this less painful */
		zs.avail_out = (uInt) zfile.uncomp_size;
		zs.next_out = (Bytef*) z_buf_out;

		inflateInit2 (&zs, -13);
		inflate (&zs, Z_NO_FLUSH);
		inflateEnd (&zs);
		{
			stream_t tmps =
				s_create_from_buf (z_buf_out, zfile.uncomp_size);

			utf16_2_ascii (&tmps, sout);

			s_free (&tmps);
		}
	}

	{

	}

	{
		/*
		*	L1: SOAP layer
		*	 -> also skip the footer, for clarity
		*/
		s_seekg (sin, + strlen (MA_SOAP_POSTFIX));
	}

	/* deinit */

	s_free (&s1);
	s_free (&s2);
	s_free (&s3);
}

/*
 *	carries out request to the schoolmaster servers and
 *	acquires and returns decoded XML data
 *	 - m: session pointer, acquired by ma_medius_init
 *	 - service: type of service to send the request to
 *	 - req: request data, will be encoded
 *	 - resp: response data, will be decoded
 */
int ma__do_request (struct ma_medius* m, const char* service, stream_t* req, stream_t* resp)
{
	/* init */
	int ret = 0;

	CURL* curl = curl_easy_init();
	char url[MAX_PATH];

	stream_t rawresp = s_create ();

	memset (url, 0, MAX_PATH);

	/* format url correctly, including service and
	 * correct version */
	sprintf (url, "https://%s/%s/WCFServices/%s",
			 m->url_base,
			 m->url_v,
			 service);

	/* setup curl to execute our request correctly,
	 * using SSL and HTTP POST data */
	curl_easy_setopt (curl, CURLOPT_URL, url);
	curl_easy_setopt (curl, CURLOPT_URL, url);
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);

	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, ma__receive_callback);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, &rawresp);

	/*
	 *	TODO: Request data, in!
	 *		  Needs to be
	 *			- XML encoded (oh... BTW... UTF-16)
	 *			- ZIPPED
	 *			- ZipCrypt encrypted
	 *			- in hexdecimal bytes
	 *			- wrapped by SOAP
	 */

	/* send request! */
	if (curl_easy_perform (curl) == CURLE_OK)
	{
		/* decode the acquired data to readable format
		 * and then write it */
		ret = ( (ma__decode_request (&rawresp, resp) == MA_OK)
				? MA_OK : MA_EMALFORMED);
	}
	else {
		ret = MA_ECONNECTION;
	}

	/* cleanup */
	s_free (&rawresp);

	return ret;
}

/*
 *	makes first contact with the medius server;
 *	verifies connection and acquires init data
 *	 - m: session ptr
 *	 - name: name of medius server (e.g. example.swp.nl)
 */
int ma_medius_init (struct ma_medius* m, const char* name)
{
	/* init */
	int ret = 0;
	int i = 0;

	CURL* curl = curl_easy_init();
	char url[MAX_URL];
	char hdr_httploc[MAX_HDR];

	llist_t lhdrs = ll_create ();

	memset (url, 0, MAX_URL);
	memset (hdr_httploc, 0, MAX_HDR);

	/* format url correctly */
	sprintf (url, "https://%s/", name);

	/* prepare a list with the Location header-var
	 * added to pass on to ma__header_callback */
	strcpy (hdr_httploc, "Location");
	ll_push (&lhdrs, hdr_httploc);

	/* setup curl to execute our request correctly,
	 * using SSL and HTTP GET */
	curl_easy_setopt (curl, CURLOPT_URL, url);
	curl_easy_setopt (curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);

	curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, ma__header_callback);
	curl_easy_setopt (curl, CURLOPT_WRITEHEADER, &lhdrs);

	/* execute request! */
	if (curl_easy_perform (curl) == CURLE_OK)
	{
		/* if the Location header has been found, we
		 * consider this request a success */
		if (strlen (hdr_httploc) > strlen ("Location"))
		{
			/* make this into a stream so we can read
			 * it more easily later */
			stream_t shdr_httploc =
				s_create_from_buf ( (uint8_t*) hdr_httploc, strlen (hdr_httploc));

			memset (m->url_base, 0, MAX_URL);
			memset (m->url_v, 0, MAX_URL);

			/* copy over base url */
			strcpy (m->url_base, name);

			/* read the version string */
			s_seekg (&shdr_httploc, + (strlen ("Location: /")));
			s_read_until (&shdr_httploc, '/', (uint8_t*) m->url_v, MAX_URL);

			s_free (&shdr_httploc);
		}
		else {
			ret = MA_EMALFORMED;
		}
	}
	else {
		ret = MA_ECONNECTION;
	}

	/* cleanup */
	ll_free (&lhdrs);

	return ret;
}

void ma_medius_delete (struct ma_medius* m)
{
	/* TODO: Remove, maybe, someday */
}