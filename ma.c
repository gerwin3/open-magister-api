#include "ma.h"

#include <zlib.h>

#include "list.h"
#include "utf16.h"
#include "hex.h"
#include "zip.h"
#include "zip_crypt.h"

/*
 *	Callback - utilized by curl, used by us - to read
 *	response data.
 *	 - buf: response data
 *	 - size: byte size
 *	 - len: num byte
 *	 - userp: Pointer provided by us; will be converted
 *			  to stream_t* to write output data.
 */
int ma__receive_callback (void* buf, size_t size, size_t len, void* userp)
{
	s_write ( (stream_t*) userp,
			  (uint8_t*) buf,
			  (size * len));

	return len;
}

/*
 *	Callback - utilized by curl, used by us - to read
 *	response headers.
 *	 - buf: header line bytes
 *	 - size: byte size
 *	 - len: num byte
 *	 - userp: Pointer provided by us; will be converted
 *			  to llist_*, containing header-var's that
 *			  are to be replaced by their values.
 */
int ma__header_callback (void* buf, size_t size, size_t len, void* userp)
{
	llist_t* lhdrs = (llist_t*) userp;
	struct llist_node* nhdr = NULL;

	/* We're trying to find out if this is one of the
	 * headers we're looking for. */
	for (nhdr = lhdrs->head;
		 nhdr != NULL;
		 nhdr = nhdr->next)
	{
		char* shdr = (char*) nhdr->v;

		if (memcmp (shdr, buf, strlen (shdr)) == 0)
		{
			/* We've found a header we need, reset the
			 * input buffer and copy over this line to
			 * it; Now it's a output buf. */
			memset (shdr, 0, MAX_HDR);
			memcpy (shdr, buf, max ( (size * len), MAX_HDR));
		}
	}

	return len;
}

/*
 *	Encodes some request data to make it readable
 *	for magister servers.
 *	 - sin: input data
 *	 - sout: output data
 */
int ma__encode_request (stream_t* sin, stream_t* sout)
{
	int r = 0;
	stream_t s1, s2, s3;

	uint8_t* z_buf_out = NULL;
	z_stream z_strm;

	s1 = s_create ();
	s2 = s_create ();
	s3 = s_create ();

	/*
	 *	L6: UTF-16 encoded
	 *	 -> converts ASCII to UTF-16
	 */
	ascii_to_utf16 (sin, &s1);

	/*
	 *	L5: DEFLATE compressed bytes
	 *	 -> compress them
	 */

	z_buf_out =
		(uint8_t*) malloc (s1.len);

	/* initialize a 'z_stream' that zlib uses to
	 * inflate/deflate this data */
	z_strm.zalloc = Z_NULL;
	z_strm.zfree = Z_NULL;
	z_strm.opaque = Z_NULL;
	z_strm.avail_in = (uInt) s1.len;
	z_strm.next_in = (Bytef*) s1.buf;
	z_strm.avail_out = (uInt) s1.len;
	z_strm.next_out = (Bytef*) z_buf_out;

	/* use zlib to deflate the data, the second
	 * argument, -13, makes this a raw deflate */
	deflateInit2 (&z_strm,
				  Z_DEFAULT_COMPRESSION,
				  Z_DEFLATED, -13, 8,
				  Z_DEFAULT_STRATEGY);

	if (deflate (&z_strm, Z_FINISH) == Z_STREAM_END)
	{
		stream_t stmp =
			s_create_from_buf (z_buf_out, z_strm.total_out);

		/*
		 *	L4: PK Zip encryption
		 *	 -> encrypt with MA_ZIP_PASSWORD
		 */
		if (zip_encrypt (&stmp, &s2, MA_ZIP_PASSWORD, MA_ZIP_MODTIME) == ZIP_CRYPT_OK)
		{
			struct zip_file_info zfile = {
				MA_ZIP_NAME,			/* => content */
				strlen (MA_ZIP_NAME),
				ZIP_FLAG_ENCRYPTED |	/* we have encrypted the data! */
				ZIP_FLAG_DATA_DESC,		/* using a data descriptor */
				ZIP_METHOD_DEFLATED,
				z_strm.total_out,		/* size of compressed data */
				s1.len,					/* and before compression */
				MA_ZIP_MODTIME,			/* is used as seed for decryption */
				0,
				0						/* TODO: This might cause problems! */
			};

			/*
			 *	L3: Zip container
			 *	 -> write the 'content' file to the zip
			 */
			if (zip_file_write (&s3, &zfile, &s2) == ZIP_OK)
			{
				/*
				 *	L1: SOAP layer
				 *	 -> write soap prefix/header
				 */
				s_write (sout, (uint8_t*) MA_SOAP_PREFIX, strlen (MA_SOAP_PREFIX));

				/*
				 *	L2: hexdec layer
				 *	 -> convert to binary with hex2bin ()
				 */
				bin2hex (&s3, sout);

				/*
				 *	L1: SOAP layer
				 *	 -> write soap postfix/footer
				 */
				s_write (sout, (uint8_t*) MA_SOAP_POSTFIX, strlen (MA_SOAP_POSTFIX));

				r = MA_OK;
			}
			else {
				r = MA_EINVAL;
			}
		}
		else {
			r = MA_EINVAL;
		}

		deflateEnd (&z_strm);

		s_free (&stmp);
	}
	else {
		r = MA_EINVAL;
	}

	free (z_buf_out);

	s_free (&s1);
	s_free (&s2);
	s_free (&s3);
}

/*
 *	Decodes data from the magister servers to a
 *	format that can easily be parsed.
 *	 - sin: input data
 *	 - sout: output data
 */
int ma__decode_request (stream_t* sin, stream_t* sout)
{
	/* init */

	int r = 0;
	stream_t s1, s2, s3, stmp;

	struct zip_file_info zfile;

	s1 = s_create ();
	s2 = s_create ();
	s3 = s_create ();

	/*
	 *	L1: SOAP layer
	 *	 -> simply skip the header
	 */
	s_seekg (sin, + strlen (MA_SOAP_PREFIX));

	/*
	 *	L2: hexdec layer
	 *	 -> convert to binary with hex2bin ()
	 */
	hex2bin (sin, &s1);

	/*
	 *	L3: Zip container
	 *	 -> extract the 'content' file from the zip
	 */
	if (zip_file_read (&s1, MA_ZIP_NAME, &zfile, &s2) == ZIP_OK)
	{
		/*
		 *	L4: PK Zip encryption
		 *	 -> decrypt with MA_ZIP_PASSWORD
		 */
		if (zip_decrypt (&s2, &s3, MA_ZIP_PASSWORD, zfile.mod_time) == ZIP_CRYPT_OK)
		{
			/*
			 *	L5: DEFLATE compressed bytes
			 *	 -> uncompress them
			 */

			uint8_t* z_buf_out =
				(uint8_t*) malloc (zfile.uncomp_size);

			/* initialize a 'z_stream' that zlib uses to
			 * inflate/deflate this data */
			z_stream z_strm;
			z_strm.zalloc = Z_NULL;
			z_strm.zfree = Z_NULL;
			z_strm.opaque = Z_NULL;
			z_strm.avail_in = (uInt) s3.len;
			z_strm.next_in = (Bytef*) s3.buf;
			z_strm.avail_out = (uInt) zfile.uncomp_size;
			z_strm.next_out = (Bytef*) z_buf_out;

			/* use zlib to inflate the data, the second
			 * argument, -13, makes this a raw inflate */
			inflateInit2 (&z_strm, -13);

			if (inflate (&z_strm, Z_NO_FLUSH) == Z_OK)
			{
				/*
				 *	L6: UTF-16 encoded
				 *	 -> we want regular ASCII for parsing
				 */
				stream_t stmp =
					s_create_from_buf (z_buf_out,
									   zfile.uncomp_size);

				utf16_to_ascii (&stmp, sout);
	
				/*
				 *	L1: SOAP layer
				 *	 -> also skip the footer, for clarity
				 */
				s_seekg (sin, + strlen (MA_SOAP_POSTFIX));

				/* seems like we made it :) */
				r = MA_OK;

				s_free (&stmp);
			}
			else {
				r = MA_EMALFORMED;
			}

			inflateEnd (&z_strm);

			free (z_buf_out);
		}
		else {
			r = MA_EMALFORMED;
		}
	}
	else {
		r = MA_EMALFORMED;
	}

	/* deinit */

	s_free (&s1);
	s_free (&s2);
	s_free (&s3);

	return r;
}

/*
 *	Carries out request to the schoolmaster servers and
 *	acquires and returns decoded XML data.
 *	 - m: Session pointer, acquired by ma_medius_init.
 *	 - service: Type of service to send the request to.
 *	 - req: Request data, will be encoded.
 *	 - resp: Response data, will be decoded.
 */
int ma__do_request (struct ma_medius* m, const char* service, stream_t* req, stream_t* resp)
{
	/* init */
	int ret = 0;
	char url[MAX_PATH];
	stream_t encreq = s_create ();
	stream_t encresp = s_create ();

	memset (url, 0, MAX_PATH);

	/* format url correctly, including service and
	 * correct version */
	sprintf (url, "https://%s/%s/WCFServices/%s",
			 m->url_base,
			 m->url_v,
			 service);

	/* Encode request data for Schoolmasters servers */
	if (ma__encode_request (req, &encreq) == MA_OK)
	{
		curl_easy_setopt (m->curl, CURLOPT_URL, url);
		curl_easy_setopt (m->curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt (m->curl, CURLOPT_SSL_VERIFYHOST, 0);

		/* Insert POST request data */
		curl_easy_setopt (m->curl, CURLOPT_POSTFIELDS,
								   (uint8_t*) s_glance (&encreq) );

		curl_easy_setopt (m->curl, CURLOPT_POSTFIELDSIZE,
								   req->len);

		/* Receive response body */
		curl_easy_setopt (m->curl, CURLOPT_WRITEFUNCTION,
								   ma__receive_callback);

		curl_easy_setopt (m->curl, CURLOPT_WRITEDATA,
								   &encresp);

		/* Send request! */
		if (curl_easy_perform (m->curl) == CURLE_OK)
		{
			ret = ( (ma__decode_request (&encresp, resp) == MA_OK)
					? MA_OK
					: MA_EMALFORMED);
		}
		else {
			ret = MA_ECONNECTION;
		}
	}
	else {
		ret = MA_EINVAL;
	}

	/* Cleanup */

	curl_easy_setopt (m->curl, CURLOPT_URL, NULL);
	curl_easy_setopt (m->curl, CURLOPT_POSTFIELDS, NULL);
	curl_easy_setopt (m->curl, CURLOPT_POSTFIELDSIZE, NULL);
	curl_easy_setopt (m->curl, CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt (m->curl, CURLOPT_WRITEDATA, NULL);

	s_free (&encreq);
	s_free (&encresp);

	return ret;
}

/*
 *	Makes first contact with the medius server;
 *	verifies connection and acquires init data.
 *	 - m: session ptr
 *	 - name: Name of medius server (e.g. example.swp.nl).
 */
int ma_medius_init (struct ma_medius* m, const char* name)
{
	/* init */
	int ret = 0;
	int i = 0;
	char url[MAX_URL];
	char hdr_httploc[MAX_HDR];
	llist_t lhdrs = ll_create ();

	memset (url, 0, MAX_URL);
	memset (hdr_httploc, 0, MAX_HDR);

	strcpy (hdr_httploc, "Location");

	sprintf (url, "https://%s/", name);

	/* list with only Location header pushed */
	ll_push (&lhdrs, hdr_httploc);

	/*
	 *	Initialize curl, the CURL session ptr is
	 *	stored with the medius, cookies etc are
	 *	persisted across requests.
	 */

	m->curl = curl_easy_init ();

	/* setup curl to execute our request correctly,
	 * using SSL and HTTP GET */
	curl_easy_setopt (m->curl, CURLOPT_URL, url);
	curl_easy_setopt (m->curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt (m->curl, CURLOPT_SSL_VERIFYHOST, 0);

	/*
	 *	Some of the cookies that will be stored:
	 *	 - LoaderMode
	 *	 - M5DB / M5DBSoort / M5VDir
	 *	 - SchoolMaster.ID
	 *	 - .ASPXAUTH
	 */
	curl_easy_setopt (m->curl, CURLOPT_COOKIEFILE, "");

	/* Runs through the header lists and replaces the
	 * variable names with their respective values */

	curl_easy_setopt (m->curl, CURLOPT_HEADERFUNCTION,
							   ma__header_callback);

	curl_easy_setopt (m->curl, CURLOPT_WRITEHEADER,
							   (void*) &lhdrs);

	/* execute request! */
	if (curl_easy_perform (m->curl) == CURLE_OK)
	{
		/* if the Location header has been found, we
		 * consider this request a success */
		if (strlen (hdr_httploc) > strlen ("Location"))
		{
			stream_t shdr_httploc =
				s_create_from_buf ( (uint8_t*) hdr_httploc,
									strlen (hdr_httploc) );

			memset (m->url_base, 0, MAX_URL);
			memset (m->url_v, 0, MAX_URL);
			strcpy (m->url_base, name);

			/* read the version string */
			s_seekg (&shdr_httploc, + (strlen ("Location: /") ) );

			s_read_until (&shdr_httploc, '/',
						  (uint8_t*) m->url_v,
						  MAX_URL);

			s_free (&shdr_httploc);
		}
		else {
			ret = MA_EMALFORMED;
		}
	}
	else {
		ret = MA_ECONNECTION;
	}

	/* Cleanup */

	curl_easy_setopt (m->curl, CURLOPT_URL, NULL);
	curl_easy_setopt (m->curl, CURLOPT_HEADERFUNCTION, NULL);
	curl_easy_setopt (m->curl, CURLOPT_WRITEHEADER, NULL);

	ll_free (&lhdrs);

	return ret;
}

/* 
 *	TODO ->
 *		Would this work? Well, no. Not yet.
 */
int ma_request_init_data (struct ma_medius *m)
{
	static const char req0[] =
		"<?xml version=\"1.0\" encoding=\"utf-16\"?>\
		<GetStartupData xmlns=\"http://tempuri.org/\">\
		<clientDate>%4d-%2d-%2dT%2d:%2d:%2d.%d+02:00</clientDate>\
		</GetStartupData>";

	int r = 0;
	char req[sizeof (req0) + 64];
	stream_t sreq = s_create ();
	stream_t sresp = s_create ();

	time_t ctime = time (NULL);
	struct tm* ptime = gmtime (&ctime);

	memset (req, 0, sizeof (req) );
	sprintf (req, req0,
			 ptime->tm_year + 1900,
			 ptime->tm_mon + 1,
			 ptime->tm_mday,
			 ptime->tm_hour,
			 ptime->tm_min,
			 ptime->tm_sec,
			 0);

	s_write (&sreq, (uint8_t*) req, strlen (req) );

	r = ma__do_request (m, MA_SVC_LOGIN, &sreq, &sresp);

	s_free (&sreq);
	s_free (&sresp);

	return r;
}