#include "zip.h"

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
			return ZIP_HDR_CEN_DIR;
		break;
		case 0x06054b50:
			return ZIP_HDR_CEN_DIR_END;
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
		case ZIP_HDR_CEN_DIR:
			return 0x02014b50;
		break;
		case ZIP_HDR_CEN_DIR_END:
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
		case ZIP_HDR_CEN_DIR:
		{
			struct zip_hdr_cen_dir* hdr =
				(struct zip_hdr_cen_dir*) hdrp;

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
		case ZIP_HDR_CEN_DIR_END:
		{
			struct zip_hdr_cen_dir_end* hdr =
				(struct zip_hdr_cen_dir_end*) hdrp;

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
		case ZIP_HDR_CEN_DIR:
		{
			struct zip_hdr_cen_dir* hdr =
				(struct zip_hdr_cen_dir*) hdrp;

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
		case ZIP_HDR_CEN_DIR_END:
		{
			struct zip_hdr_cen_dir_end* hdr =
				(struct zip_hdr_cen_dir_end*) hdrp;

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

int zip__find_parse_cen_dir (stream_t* s, struct zip_hdr_cen_dir* hdr_cendir)
{
	int i = 0;

	s_seekg (s, S_SEEK_END);
	s_seekg (s, - ((int) sizeof (struct zip_hdr_cen_dir_end) - 1));

	/* we'll try finding the end of central directory
	 * header by looping back from the end and looking
	 * for it's signature */
	for (i = s_tellg (s); i > 0; i--)
	{
		int hdrtype =
			zip__peek_hdr_type (s);

		if (hdrtype == ZIP_HDR_CEN_DIR_END || hdrtype == ZIP_HDR_CEN_DIR)
		{
			int eofs = 0;	
			
			/* we've found the cen dir end first, it can guide
			 * us to the cen dir directly. */
			if (hdrtype == ZIP_HDR_CEN_DIR_END)
			{
				struct zip_hdr_cen_dir_end hdr_cendirend;

				if (zip__read_hdr (s, (zip_hdr_p) &hdr_cendirend) != ZIP_OK) {
					return ZIP_EMALFORMED;
				}

				/* use the cen_dir_end header to find the actual
				 * central directory */
				s_seekg (s, S_SEEK_BEGIN);
				s_seekg (s, +hdr_cendirend.cendir_off);
			}

			if (zip__read_hdr (s, (zip_hdr_p) hdr_cendir) != ZIP_OK) {
				return ZIP_EMALFORMED;
			}

			return ZIP_OK;
		}
		else
		{
			/* seek back to the start minus 1 for the
			 * to process the position before this one */
			s_seekg (s, -1);
		}
	}

	return ZIP_ENOTFOUND;
}

int zip__parse_file (stream_t* s, struct zip_file_info* finfo, stream_t* fs)
{
	struct zip_hdr_local_file hdr_file;

	if (zip__read_hdr (s, (zip_hdr_p) &hdr_file) != ZIP_OK) {
		return ZIP_EMALFORMED;
	}

	/* copy over essential members ... */
	finfo->fname = hdr_file.fname;
	finfo->fname_len = hdr_file.fname_len;
	finfo->flags = hdr_file.flags;
	finfo->comp_method = hdr_file.method;
	finfo->crc32 = hdr_file.crc32;
	finfo->mod_time = hdr_file.mod_time;
	finfo->mod_date = hdr_file.mod_date;
	finfo->comp_size = hdr_file.comp_size;
	finfo->uncomp_size = hdr_file.uncomp_size;

	/* some of the data we need might be hidden in the
	 * data descriptor, if available */
	if (hdr_file.flags & ZIP_FLAG_DATA_DESC)
	{
		int i = 0;
		for (i = 0; !s_eof (s); i++, s_seekg (s, +1))
		{
			/* found the data descriptor yet? */
			if (zip__peek_hdr_type (s) == ZIP_HDR_DATA_DESC)
			{
				struct zip_hdr_data_desc* hdr_ddesc_test =
					(struct zip_hdr_data_desc*) s_glance (s);

				/* try to validate the header, to make it very
				 * unlikely we ever run into a collision */
				if (hdr_ddesc_test->comp_size == i)
				{
					struct zip_hdr_data_desc hdr_ddesc;

					/* this is definitely the header we're looking
					 * for... parse */
					if (zip__read_hdr (s, (zip_hdr_p) &hdr_ddesc) != 0) {
						return -1;
					}

					/* these are the ones we need! */
					finfo->comp_size = hdr_ddesc.comp_size;
					finfo->uncomp_size = hdr_ddesc.uncomp_size;
					finfo->crc32 = hdr_ddesc.crc32;

					/* as if it never happened */
					s_seekg (s, - (int) (i + sizeof (struct zip_hdr_data_desc)));

					break;
				}
			}
		}

		if (finfo->comp_size > 0 && finfo->uncomp_size)
		{
			/* write the - still compressed - data to the
			 * provided output stream */
			s_write (fs, s_glance (s), finfo->comp_size);

			/* we just took some data from the input stream
			 * without shifting the get ptr; do it now */
			s_seekg (s, finfo->comp_size);

			return 0;
		}
		else {
			return ZIP_EMALFORMED;
		}
	}
}

/*
 *	will read an parse the entire zip stream to look for one
 *	file to retrieve its headers and bytes.
 *	 - s: stream to read from
 *	 - fname: name of file to find
 *	 - finfo: [out] file info
 *	 - fs: [out] output stream for file contents
 */
int zip_file_read(stream_t* s, const char* fname, struct zip_file_info* finfo, stream_t* fs)
{
	int eofs = 0;
	struct zip_hdr_cen_dir hdr_cendir;

	/* first, find the central directory header, it
	 * will guide us the rest of the parsing process */
	if (zip__find_parse_cen_dir (s, &hdr_cendir) != ZIP_OK) {
		return ZIP_EMALFORMED;
	}

	/* locate and read the first local file header */
	s_seekg (s, S_SEEK_BEGIN);
	s_seekg (s, +hdr_cendir.fhdr_off);

	while (!eofs)
	{
		/* we can find out what header we're currently
		 * onto, this only works because the parsing
		 * functions always seek until directly after
		 * their last block */
		switch (zip__peek_hdr_type (s))
		{
			case ZIP_HDR_LOCAL_FILE:
			{
				/* it seems to be a file, parse it! */
				if (zip__parse_file (s, finfo, fs) == ZIP_OK)
				{
					if (memcmp (fname, finfo->fname, finfo->fname_len) == 0)
					{
						/* yes, we found the file! */
						return ZIP_OK;
					}
					else {
						/* reset memory to prevent confusion ;) */
						memset (finfo, 0, sizeof (struct zip_file_info));
					}
				}
				else {
					return ZIP_EMALFORMED;
				}
			}
			break;
			case ZIP_HDR_CEN_DIR:
			case ZIP_HDR_CEN_DIR_END:
				/* when the cen (end) dir header is found, there
				 * will be no more local file headers to come. */
				eofs = 1;
			break;
			default:
				/* unexpected! */
				eofs = 1;
		}
	}

	return ZIP_ENOTFOUND;
}

/*
 *	write a zip file with a single file inside
 *	 - s: [out] stream to write to
 *	 - finfo: file information
 *	 - fs: input stream to read file contents
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
		finfo->crc32,
		finfo->comp_size,
		finfo->uncomp_size,
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

	/* compensate for glancing on the in stream */
	s_seekg (s, finfo->comp_size);

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
		struct zip_hdr_cen_dir hdr_cendir = {
			zip__hdr_sign (ZIP_HDR_CEN_DIR),
			0,
			0,
			finfo->flags,
			finfo->comp_method,
			finfo->mod_time,
			finfo->mod_date,
			finfo->crc32,
			finfo->comp_size,
			finfo->uncomp_size,
			finfo->fname_len,
			0,
			0,
			0,
			0,
			0,
			0
		};

		if (zip__write_hdr (s, (zip_hdr_p) &hdr_cendir) != ZIP_OK) {
			return ZIP_EINVAL;
		}
	}

	/* wrote:
	 *	- local file header
	 *	- file data
	 *	- central directory header
	 * => done! */
	return ZIP_OK;
}