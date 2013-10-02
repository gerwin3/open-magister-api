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

struct zip_hdr_cen_dir
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

struct zip_hdr_cen_dir_end
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

int zip__peek_hdr_type (strm_t* s)
{
	zip_hdr_p hdr_base =
		(zip_hdr_p) s_glance (s);

	return zip__hdr_type (hdr_base);
}

int zip__read_header (strm_t* s, zip_hdr_p hdrp)
{
	int hdrtype =
		zip__peek_hdr_type (s);

	if (hdrtype == ZIP_HDR_UNKOWN) {
		return -1;
	}

	switch (hdrtype)
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
	}

	return 0;
}

/*
 *	will read an parse the entire zip stream to look for one
 *	file and retrieve its headers and bytes.
 *	 - s: stream to read from
 *	 - fname: name of file to find
 *	 - finfo: [out] file info
 *	 - fs: [out] output stream for file contents
 */
int zip_file_find(strm_t* s, const char* fname, struct zip_file_info* finfo, strm_t* fs)
{
	int i = 0;

	s_seekg (s, S_SEEK_BEGIN);
	s_seekg (s, s->len - sizeof (struct zip_hdr_cen_dir_end) - 1);

	/* we'll try finding the end of central directory
	 * header by looping back from the end and looking
	 * for it's signature */

	for (i = s_tellg (s); i > 0; i--)
	{
		int hdrtype =
			zip__peek_hdr_type (s);

		/* found the cen_dir_end header?? */
		if (hdrtype == ZIP_HDR_CEN_DIR_END || hdrtype == ZIP_HDR_CEN_DIR)
		{
			int eofs = 0;	
			struct zip_hdr_cen_dir hdr_cendir;
			
			/* we've found the cen dir end first, it can guide
			 * us to the cen dir directly. */
			if (hdrtype == ZIP_HDR_CEN_DIR_END)
			{
				struct zip_hdr_cen_dir_end hdr_cendirend;

				if (zip__read_header (s, (zip_hdr_p) &hdr_cendirend) != 0) {
					return -1;
				}

				/* use the cen_dir_end header to find the actual
				 * central directory */
				s_seekg (s, S_SEEK_BEGIN);
				s_seekg (s, +hdr_cendirend.cendir_off);
			}

			if (zip__read_header (s, (zip_hdr_p) &hdr_cendir) != 0) {
				return -1;
			}

			/* locate and read the first local file header */
			s_seekg (s, S_SEEK_BEGIN);
			s_seekg (s, +hdr_cendir.fhdr_off);

			while (!eofs)
			{
				switch (zip__peek_hdr_type (s))
				{
					case ZIP_HDR_LOCAL_FILE:
					{
						struct zip_hdr_local_file hdr_file;

						if (zip__read_header (s, (zip_hdr_p) &hdr_file) != 0) {
							return -1;
						}

						if (memcmp (fname, hdr_file.fname, hdr_file.fname_len) == 0)
						{
							finfo->fname = fname;
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
								for (i = 0; !s_eof (s, S_MODE_G); i++, s_seekg (s, +1))
								{
									if (zip__peek_hdr_type (s) == ZIP_HDR_DATA_DESC)
									{
										struct zip_hdr_data_desc* hdr_ddesc_test =
											(struct zip_hdr_data_desc*) s_glance (s);

										if (hdr_ddesc_test->comp_size == i)
										{
											struct zip_hdr_data_desc hdr_ddesc;

											/* this is definitely the header we're looking
											 * for... parse */

											if (zip__read_header (s, (zip_hdr_p) &hdr_ddesc) != 0) {
												return -1;
											}

											finfo->comp_size = hdr_ddesc.comp_size;
											finfo->uncomp_size = hdr_ddesc.uncomp_size;
											finfo->crc32 = hdr_ddesc.crc32;

											/* as if it never happened */
											s_seekg (s, - (i + sizeof (struct zip_hdr_data_desc)));

											break;
										}
									}
								}
							}

							if (finfo->comp_size > 0)
							{
								s_write (fs, s_glance (s), finfo->comp_size);
								s_seekg (s, finfo->comp_size);

								return 0;
							}
							else
							{
								return -1; /* TODO: Error malformed */
							}
						}
					}
					break;
					case ZIP_HDR_CEN_DIR:
						/* when the central directory header is found there
						 * will be no more local file headers to come. */
						eofs = 1;
					break;
					default:
						/* unexpected! */
						eofs = 1;
				}
			}
		}
		else
		{
			/* seek back to the start minus 1 for the
			 * to process the position before this one */
			s_seekg (s, -1);
		}
	}

	return -2; /* TODO error not found! */
}

