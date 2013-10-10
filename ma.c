#include "ma.h"

#include <curl/curl.h>
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
 *	Encodes some request data to make it readable
 *	for magister servers.
 *	 - sin: input data
 *	 - sout: output data
 */
int ma__encode_request (stream_t* sin, stream_t* sout)
{
	int r = 0;
	stream_t s1, s2, s3;

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

	uint8_t* z_buf_out =
		(uint8_t*) malloc (sin->len);

	/* initialize a 'z_stream' that zlib uses to
	 * inflate/deflate this data */
	z_stream z_strm;
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

	if (deflate (&z_strm, Z_NO_FLUSH) == Z_OK)
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
			 *	 -> extract the 'content' file from the zip
			 */
			if (zip_file_write (&s2, &zfile, &s3) == ZIP_OK)
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
				stmp = s_create_from_buf (z_buf_out,
										  zfile.uncomp_size);

				utf16_to_ascii (&stmp, sout);
	
				/*
				 *	L1: SOAP layer
				 *	 -> also skip the footer, for clarity
				 */
				s_seekg (sin, + strlen (MA_SOAP_POSTFIX));

				/* seems like we made it :) */
				r = MA_OK;
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

	s_free (&stmp);
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
		/* setup curl to execute our request correctly,
		 * using SSL and HTTP POST data */
		curl_easy_setopt (m->curl, CURLOPT_URL, url);
		curl_easy_setopt (m->curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt (m->curl, CURLOPT_SSL_VERIFYHOST, 0);

		curl_easy_setopt (m->curl, CURLOPT_POSTFIELDS, (uint8_t*) s_glance (&encreq) );
		curl_easy_setopt (m->curl, CURLOPT_POSTFIELDSIZE, req->len);

		curl_easy_setopt (m->curl, CURLOPT_WRITEFUNCTION, ma__receive_callback);
		curl_easy_setopt (m->curl, CURLOPT_WRITEDATA, &encresp);

		/* send request! */
		if (curl_easy_perform (m->curl) == CURLE_OK)
		{
			/* decode the acquired data to readable format
			 * and then write it */
			ret = ( (ma__decode_request (&encresp, resp) == MA_OK)
					? MA_OK : MA_EMALFORMED);
		}
		else {
			ret = MA_ECONNECTION;
		}
	}
	else {
		ret = MA_EINVAL;
	}

	/* cleanup */
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

	/* Initialize CURL, the same session is used the whole
	 * time so cookies etc are persisted. */
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
	curl_easy_setopt (m->curl, CURLOPT_HEADERFUNCTION, ma__header_callback);
	curl_easy_setopt (m->curl, CURLOPT_WRITEHEADER, &lhdrs);

	/* execute request! */
	if (curl_easy_perform (m->curl) == CURLE_OK)
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