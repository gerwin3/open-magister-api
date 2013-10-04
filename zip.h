#ifndef ZIP_H
#define ZIP_H

#include <stdio.h>
#include <string.h>

#include "stream.h"

#define ZIP_OK					0
#define ZIP_EINVAL				1
#define ZIP_EMALFORMED			2
#define ZIP_ENOTFOUND			3

#define ZIP_MAGIC				0x00004b50

#define ZIP_HDR_UNKOWN			0
#define ZIP_HDR_LOCAL_FILE		1
#define ZIP_HDR_DATA_DESC		2
#define ZIP_HDR_CEN_DIR			3
#define ZIP_HDR_CEN_DIR_END		4

#define ZIP_FLAG_ENCRYPTED		0x0001
#define ZIP_FLAG_OPT_1			0x0002
#define ZIP_FLAG_OPT_2			0x0004
#define ZIP_FLAG_DATA_DESC		0x0008
#define ZIP_FLAG_ENHANCED		0x0010
#define ZIP_FLAG_PATCHED		0x0020
#define ZIP_FLAG_CRYPT_STRONG	0x0040
#define ZIP_FLAG_LANG_ENC		0x0800
#define ZIP_FLAG_MASK			0x2000

#define ZIP_METHOD_NONE			0
#define ZIP_METHOD_SHRUNK		1
#define ZIP_METHOD_REDUCED_F1	2
#define ZIP_METHOD_REDUCED_F2	3
#define ZIP_METHOD_REDUCED_F3	4
#define ZIP_METHOD_REDUCED_F4	5
#define ZIP_METHOD_IMPLODED		6
#define ZIP_METHOD_DEFLATED		8
#define ZIP_METHOD_ENH_DEFLATED	9
#define ZIP_METHOD_PKIMPLODED	10
#define ZIP_METHOD_BZIP2		12
#define ZIP_METHOD_LZMA			14
#define ZIP_METHOD_IBMTERSE		18
#define ZIP_METHOD_IBMLZ77		19
#define ZIP_METHOD_PPMD			98

struct zip_file_info
{
	const char* fname;	/* backed by the input stream, freeing the stream
						 * will invalidate this string */
	int fname_len;
	int comp_method;
	int comp_size;
	int uncomp_size;
	int mod_time;
	int mod_date;
	uint32_t crc32;
};

extern int zip_file_find (stream_t* s, const char* fname, struct zip_file_info* finfo, stream_t* fs);

#endif