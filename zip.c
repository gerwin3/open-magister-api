#include "zip.h"

#include "common.h"

typedef struct zip_hdr_base* zip_hdr_p;

struct zip_hdr_base
{
	uint32_t sign;		/* common signature, used to identify
						 * type of hdr */
};

struct zip_hdr_local_file
{
	uint32_t sign;
	uint16_t reqv;
	uint16_t flags;
	uint16_t method;
	uint16_t mod_time;
	uint16_t mod_date;
	uint32_t crc32;
	uint32_t comp_size;
	uint32_t uncomp_size;
	uint16_t fname_len;
	uint16_t extra_len;
	
	char* fname;
	uint8_t* extra;		/* these are backed by the buffer from
						 * the stream and thus managed by the user */
};

struct zip_hdr_data_desc
{
	uint32_t sign;
	uint32_t crc32;
	uint32_t comp_size;
	uint32_t uncomp_size;
};

struct zip_hdr_cendir_file
{
	uint32_t sign;
	uint16_t madev;
	uint16_t reqv;
	uint16_t flags;
	uint16_t method;
	uint16_t mod_time;
	uint16_t mod_date;
	uint32_t crc32;
	uint32_t comp_size;
	uint32_t uncomp_size;
	uint16_t fname_len;
	uint16_t extra_len;
	uint16_t fcomm_len;
	uint16_t ndisk_start;
	uint16_t finattrs;
	uint32_t fexattrs;
	uint32_t fhdr_off;

	char* fname;
	uint8_t* extra;
	char* fcomm;		/* these are backed by the buffer from
						 * the stream and thus managed by the user */
};

struct zip_hdr_cendir_end
{
	uint32_t sign;
	uint16_t ndisk;
	uint16_t ndisk_start;
	uint16_t ncendirs;
	uint32_t cendir_len;
	uint32_t cendir_off;
	uint16_t comm_len;

	char* comm;			/* backed by stream buffer */
};

int zip__hdr_type (const zip_hdr_p hdrp)
{
	/* determine type of header on magic signature;
	 * the first 4 bytes */
	switch (hdrp->sign)
	{
		case 0x04034b50:
			return ZIP_HDR_LOCAL_FILE;
		break;
		case 0x08074b50:
			return ZIP_HDR_DATA_DESC;
		break;
		case 0x02014b50:
			return ZIP_HDR_CENDIR_FILE;
		break;
		case 0x06054b50:
			return ZIP_HDR_CENDIR_END;
		break;
		default:
			return ZIP_HDR_UNKOWN;
	}
}

int zip__hdr_sign (int hdrtype)
{
	/* find the signature by hdrtype */
	switch (hdrtype)
	{
		case ZIP_HDR_LOCAL_FILE:
			return 0x04034b50;
		break;
		case ZIP_HDR_DATA_DESC:
			return 0x08074b50;
		break;
		case ZIP_HDR_CENDIR_FILE:
			return 0x02014b50;
		break;
		case ZIP_HDR_CENDIR_END:
			return 0x06054b50;
		break;
		default:
			return ZIP_EINVAL;
	}
}

int zip__peek_hdr_type (stream_t* s)
{
	zip_hdr_p hdr_base =
		(zip_hdr_p) s_glance (s);

	return zip__hdr_type (hdr_base);
}

int zip__read_hdr (stream_t* s, zip_hdr_p hdrp)
{
	switch (zip__peek_hdr_type (s))
	{
		case ZIP_HDR_LOCAL_FILE:
		{
			struct zip_hdr_local_file* hdr =
				(struct zip_hdr_local_file*) hdrp;

			hdr->sign = s_read_int32 (s);
			hdr->reqv = s_read_int16 (s);
			hdr->flags = s_read_int16 (s);
			hdr->method = s_read_int16 (s);
			hdr->mod_time = s_read_int16 (s);
			hdr->mod_date = s_read_int16 (s);
			hdr->crc32 = s_read_int32 (s);
			hdr->comp_size = s_read_int32 (s);
			hdr->uncomp_size = s_read_int32 (s);
			hdr->fname_len = s_read_int16 (s);
			hdr->extra_len = s_read_int16 (s);
			
			hdr->fname = (char*) s_glance (s);
			s_seekg (s, +hdr->fname_len);

			hdr->extra = s_glance (s);
			s_seekg (s, +hdr->extra_len);
		}
		break;
		case ZIP_HDR_DATA_DESC:
		{
			struct zip_hdr_data_desc* hdr =
				(struct zip_hdr_data_desc*) hdrp;

			hdr->sign = s_read_int32 (s);
			hdr->crc32 = s_read_int32 (s);
			hdr->comp_size = s_read_int32 (s);
			hdr->uncomp_size = s_read_int32 (s);
		}
		break;
		case ZIP_HDR_CENDIR_FILE:
		{
			struct zip_hdr_cendir_file* hdr =
				(struct zip_hdr_cendir_file*) hdrp;

			hdr->sign = s_read_int32 (s);
			hdr->madev = s_read_int16 (s);
			hdr->reqv = s_read_int16 (s);
			hdr->flags = s_read_int16 (s);
			hdr->method = s_read_int16 (s);
			hdr->mod_time = s_read_int16 (s);
			hdr->mod_date = s_read_int16 (s);
			hdr->crc32 = s_read_int32 (s);
			hdr->comp_size = s_read_int32 (s);
			hdr->uncomp_size = s_read_int32 (s);
			hdr->fname_len = s_read_int16 (s);
			hdr->extra_len = s_read_int16 (s);
			hdr->fcomm_len = s_read_int16 (s);
			hdr->ndisk_start = s_read_int16 (s);
			hdr->finattrs = s_read_int16 (s);
			hdr->fexattrs = s_read_int32 (s);
			hdr->fhdr_off = s_read_int32 (s);

			hdr->fname = (char*) s_glance (s);
			s_seekg (s, +hdr->fname_len);

			hdr->extra = s_glance (s);
			s_seekg (s, +hdr->extra_len);

			hdr->fcomm = (char*) s_glance (s);
			s_seekg (s, +hdr->fcomm_len);
		}
		break;
		case ZIP_HDR_CENDIR_END:
		{
			struct zip_hdr_cendir_end* hdr =
				(struct zip_hdr_cendir_end*) hdrp;

			hdr->sign = s_read_int32 (s);
			hdr->ndisk = s_read_int16 (s);
			hdr->ndisk_start = s_read_int16 (s);
			hdr->ncendirs = s_read_int16 (s);
			hdr->cendir_len = s_read_int32 (s);
			hdr->cendir_off = s_read_int32 (s);
			hdr->comm_len = s_read_int16 (s);

			hdr->comm = (char*) s_glance (s);
			s_seekg (s, +hdr->comm_len);
		}
		break;
		default:
			return ZIP_EMALFORMED;
	}

	return ZIP_OK;
}

int zip__write_hdr (stream_t* s, zip_hdr_p hdrp)
{
	switch (zip__hdr_type (hdrp))
	{
		case ZIP_HDR_LOCAL_FILE:
		{
			struct zip_hdr_local_file* hdr =
				(struct zip_hdr_local_file*) hdrp;

			s_write_int32 (s, hdr->sign);
			s_write_int16 (s, hdr->reqv);
			s_write_int16 (s, hdr->flags);
			s_write_int16 (s, hdr->method);
			s_write_int16 (s, hdr->mod_time);
			s_write_int16 (s, hdr->mod_date);
			s_write_int32 (s, hdr->crc32);
			s_write_int32 (s, hdr->comp_size);
			s_write_int32 (s, hdr->uncomp_size);
			s_write_int16 (s, hdr->fname_len);
			s_write_int16 (s, hdr->extra_len);
			
			s_write (s, (uint8_t*) hdr->fname, hdr->fname_len);
			s_write (s, hdr->extra, hdr->extra_len);
		}
		break;
		case ZIP_HDR_DATA_DESC:
		{
			struct zip_hdr_data_desc* hdr =
				(struct zip_hdr_data_desc*) hdrp;

			s_write_int32 (s, hdr->sign);
			s_write_int32 (s, hdr->crc32);
			s_write_int32 (s, hdr->comp_size);
			s_write_int32 (s, hdr->uncomp_size);
		}
		break;
		case ZIP_HDR_CENDIR_FILE:
		{
			struct zip_hdr_cendir_file* hdr =
				(struct zip_hdr_cendir_file*) hdrp;

			s_write_int32 (s, hdr->sign);
			s_write_int16 (s, hdr->madev);
			s_write_int16 (s, hdr->reqv);
			s_write_int16 (s, hdr->flags);
			s_write_int16 (s, hdr->method);
			s_write_int16 (s, hdr->mod_time);
			s_write_int16 (s, hdr->mod_date);
			s_write_int32 (s, hdr->crc32);
			s_write_int32 (s, hdr->comp_size);
			s_write_int32 (s, hdr->uncomp_size);
			s_write_int16 (s, hdr->fname_len);
			s_write_int16 (s, hdr->extra_len);
			s_write_int16 (s, hdr->fcomm_len);
			s_write_int16 (s, hdr->ndisk_start);
			s_write_int16 (s, hdr->finattrs);
			s_write_int32 (s, hdr->fexattrs);
			s_write_int32 (s, hdr->fhdr_off);

			s_write (s, (uint8_t*) hdr->fname, hdr->fname_len);
			s_write (s, hdr->extra, hdr->extra_len);
			s_write (s, (uint8_t*) hdr->fcomm, hdr->fcomm_len);
		}
		break;
		case ZIP_HDR_CENDIR_END:
		{
			struct zip_hdr_cendir_end* hdr =
				(struct zip_hdr_cendir_end*) hdrp;

			s_write_int32 (s, hdr->sign);
			s_write_int16 (s, hdr->ndisk);
			s_write_int16 (s, hdr->ndisk_start);
			s_write_int16 (s, hdr->ncendirs);
			s_write_int32 (s, hdr->cendir_len);
			s_write_int32 (s, hdr->cendir_off);
			s_write_int16 (s, hdr->comm_len);

			s_write (s, (uint8_t*) hdr->comm, hdr->comm_len);
		}
		break;
		default:
			return ZIP_EMALFORMED;
	}

	return ZIP_OK;
}

int zip__seek_next_header (stream_t* s)
{
	while (!s_eof (s))
	{
		if (zip__peek_hdr_type (s) == ZIP_HDR_UNKOWN) {
			s_seekg (s, +1);
		} else {
			/* found a valid header signature! */
			break;
		}
	}

	/* if we've read the whole stream we failed to
	 * find a valid header */
	return (s_eof (s) ? ZIP_ENOTFOUND : ZIP_OK);
}

/*
 *	Will read an parse the entire zip stream to look for one
 *	file to retrieve its headers and bytes.
 *	 - s: Stream to read from.
 *	 - fname: Name of file to find.
 *	 - finfo: [out] file info
 *	 - fs: [out] Output stream for file contents.
 */
int zip_file_read(stream_t* s, const char* fname, struct zip_file_info* finfo, stream_t* fs)
{
	while (!s_eof (s))
	{
		/* seek the stream to right before the next header;
		 * will mostly yield immediately */
		zip__seek_next_header (s);

		switch (zip__peek_hdr_type (s))
		{
			case ZIP_HDR_CENDIR_FILE:
			{
				struct zip_hdr_cendir_file hdr_cdfile;
				int match = 0;
				int fpos = 0;

				if (zip__read_hdr (s, (zip_hdr_p) &hdr_cdfile) != ZIP_OK) {
					return ZIP_EMALFORMED;
				}

				/* decide if this is a match by comparing the
				 * target fname with the current file name */
				match = (memcmp (hdr_cdfile.fname,
								 fname,
								 imin(hdr_cdfile.fname_len, strlen (fname))) == 0) ? 1 : 0;

				if (match)
				{
					/* copy over essential members ... */
					finfo->fname = hdr_cdfile.fname;
					finfo->fname_len = hdr_cdfile.fname_len;
					finfo->flags = hdr_cdfile.flags;
					finfo->comp_method = hdr_cdfile.method;
					finfo->crc32 = hdr_cdfile.crc32;
					finfo->mod_time = hdr_cdfile.mod_time;
					finfo->mod_date = hdr_cdfile.mod_date;
					finfo->comp_size = hdr_cdfile.comp_size;
					finfo->uncomp_size = hdr_cdfile.uncomp_size;
				}

				/* store position of file date, we might need it
				 * later on! */
				fpos = s_tellg (s);

				/* some of the data we need might be hidden in the
				 * data descriptor, if available */
				if (hdr_cdfile.flags & ZIP_FLAG_DATA_DESC)
				{
					zip__seek_next_header (s);

					/* found the data descriptor yet? */
					if (zip__peek_hdr_type (s) == ZIP_HDR_DATA_DESC)
					{
						struct zip_hdr_data_desc hdr_ddesc;
						if (zip__read_hdr (s, (zip_hdr_p) &hdr_ddesc) != 0) {
							return -1;
						}

						if (match)
						{
							/* these are the ones we need! */
							finfo->comp_size = hdr_ddesc.comp_size;
							finfo->uncomp_size = hdr_ddesc.uncomp_size;
							finfo->crc32 = hdr_ddesc.crc32;
						}
					}
					else {
						return ZIP_EMALFORMED;
					}
				}

				/* only do the copying if we need to! */
				if (match)
				{
					/* rewind to the file data! */
					s_seekg (s, S_SEEK_BEGIN);
					s_seekg (s, +fpos);

					/* write the - still compressed - data to the
					 * provided output stream */
					s_write (fs, s_glance (s), finfo->comp_size);

					/* we just took some data from the input stream
					 * without shifting the get ptr; do it now */
					s_seekg (s, finfo->comp_size);

					return ZIP_OK;
				}
			}
			case ZIP_HDR_LOCAL_FILE:
			{
				struct zip_hdr_local_file hdr_lfile;
				int match = 0;
				int fpos = 0;

				if (zip__read_hdr (s, (zip_hdr_p) &hdr_lfile) != ZIP_OK) {
					return ZIP_EMALFORMED;
				}

				/* decide if this is a match by comparing the
				 * target fname with the current file name */
				match = (memcmp (hdr_lfile.fname,
								 fname,
								 imin (hdr_lfile.fname_len, strlen (fname))) == 0) ? 1 : 0;

				if (match)
				{
					/* copy over essential members ... */
					finfo->fname = hdr_lfile.fname;
					finfo->fname_len = hdr_lfile.fname_len;
					finfo->flags = hdr_lfile.flags;
					finfo->comp_method = hdr_lfile.method;
					finfo->crc32 = hdr_lfile.crc32;
					finfo->mod_time = hdr_lfile.mod_time;
					finfo->mod_date = hdr_lfile.mod_date;
					finfo->comp_size = hdr_lfile.comp_size;
					finfo->uncomp_size = hdr_lfile.uncomp_size;
				}

				/* store position of file date, we might need it
				 * later on! */
				fpos = s_tellg (s);

				/* some of the data we need might be hidden in the
				 * data descriptor, if available */
				if (hdr_lfile.flags & ZIP_FLAG_DATA_DESC)
				{
					zip__seek_next_header (s);

					/* found the data descriptor yet? */
					if (zip__peek_hdr_type (s) == ZIP_HDR_DATA_DESC)
					{
						struct zip_hdr_data_desc hdr_ddesc;
						if (zip__read_hdr (s, (zip_hdr_p) &hdr_ddesc) != 0) {
							return -1;
						}

						if (match)
						{
							/* these are the ones we need! */
							finfo->comp_size = hdr_ddesc.comp_size;
							finfo->uncomp_size = hdr_ddesc.uncomp_size;
							finfo->crc32 = hdr_ddesc.crc32;
						}
					}
					else {
						return ZIP_EMALFORMED;
					}
				}

				/* only do the copying if we need to! */
				if (match)
				{
					/* rewind to the file data! */
					s_seekg (s, S_SEEK_BEGIN);
					s_seekg (s, +fpos);

					/* write the - still compressed - data to the
					 * provided output stream */
					s_write (fs, s_glance (s), finfo->comp_size);

					/* we just took some data from the input stream
					 * without shifting the get ptr; do it now */
					s_seekg (s, finfo->comp_size);

					return ZIP_OK;
				}				
			}
			case ZIP_HDR_CENDIR_END:
			case ZIP_HDR_DATA_DESC:
				/* just ignore the other headers :) */
				continue;
			default:
				return ZIP_EMALFORMED;
		}
	}

	return ZIP_ENOTFOUND;
}

/*
 *	Write a zip file with a single file inside.
 *	 - s: [out] Stream to write to.
 *	 - finfo: file information
 *	 - fs: Input stream to read file contents.
 */
int zip_file_write (stream_t* s, struct zip_file_info* finfo, stream_t* fs)
{
	/* construct and write the local file header */
	struct zip_hdr_local_file hdr_file = {
		zip__hdr_sign (ZIP_HDR_LOCAL_FILE),
		0,
		finfo->flags,
		finfo->comp_method,
		finfo->mod_time,
		finfo->mod_date,
		finfo->crc32,		/* TODO: Set to zero? */
		finfo->comp_size,	/* ~ */
		finfo->uncomp_size,	/* ~ */
		finfo->fname_len,
		0,
		finfo->fname,
		NULL
	};

	if (zip__write_hdr (s, (zip_hdr_p) &hdr_file) != ZIP_OK) {
		return ZIP_EINVAL;
	}

	/* write the input data - should be compressed
	 * already - to the output stream */
	s_write (s, s_glance (fs), finfo->comp_size);

	/* compensate for glancing on the fs stream */
	s_seekg (fs, finfo->comp_size);

	/* maybe we need to use a data descriptor? */
	if (finfo->flags & ZIP_FLAG_DATA_DESC)
	{
		struct zip_hdr_data_desc hdr_ddesc = {
			zip__hdr_sign (ZIP_HDR_DATA_DESC),
			finfo->crc32,
			finfo->comp_size,
			finfo->uncomp_size
		};

		/* these ought to be zero when they are
		 * already in the data descriptor */
		hdr_file.crc32 = 0;
		hdr_file.comp_size = 0;
		hdr_file.uncomp_size = 0;

		if (zip__write_hdr (s, (zip_hdr_p) &hdr_ddesc) != ZIP_OK) {
			return ZIP_EINVAL;
		}
	}

	{
		/* construct and write the central directory */
		struct zip_hdr_cendir_end hdr_cdend = {
			zip__hdr_sign (ZIP_HDR_CENDIR_END),
			0,
			0,
			1,				/* number of cendirs: 1 */
			0,
			s_tellp (s),	/* the offset to the cendir is the start
							 * of the cd_end hdr, because we don't
							 * have any entries in it. */
			0,
			NULL
		};

		if (zip__write_hdr (s, (zip_hdr_p) &hdr_cdend) != ZIP_OK) {
			return ZIP_EINVAL;
		}
	}

	/* wrote:
	 *	- local file header
	 *	- file data
	 *	- end of central dir
	 * => done! */
	return ZIP_OK;
}