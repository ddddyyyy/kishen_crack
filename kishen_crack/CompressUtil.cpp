#include "CompressUtil.h"
#include <cassert>
#include <iostream>

unsigned char* CompressUtil::uncmp_deflate_without_header(unsigned char* data, uint32_t data_len, uint32_t uncmp_len)
{
	unsigned char* output = NULL;
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	int res = inflateInit2(&strm, -MAX_WBITS);

	if (res != Z_OK) {
		return NULL;
	}

	strm.next_in = data;
	strm.avail_in = data_len;

	output = new unsigned char[uncmp_len];
	strm.avail_out = uncmp_len;
	strm.next_out = output;


	res = inflate(&strm, Z_FULL_FLUSH);

	if (res != Z_OK && res != Z_STREAM_END) {
		goto RET2;
	}

	res = inflateEnd(&strm);

	if (res == Z_OK) {
		return output;
	}
	else {
	RET2:	delete[]output;
		return NULL;
	}
}



unsigned char* CompressUtil::cmp_deflate_without_header(unsigned char* data, uint32_t uncmp_len, uint32_t* result_len)
{
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	int res = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);

	if (res != Z_OK) {
		return NULL;
	}

	unsigned char* result = new unsigned char[uncmp_len];

	strm.next_in = data;
	strm.avail_in = uncmp_len;

	strm.next_out = result;
	strm.avail_out = uncmp_len;


	res = deflate(&strm, Z_FINISH);

	if (res != Z_OK && res != Z_STREAM_END) {
		goto RET2;
	}

	res = deflateEnd(&strm);

	if (res == Z_OK) {
		*result_len = strm.total_out;
		return result;
	}
	else {
	RET2:	delete[]result;
		return NULL;
	}
}


unsigned char* CompressUtil::lzss_decompress(
	const unsigned char* input,
	const size_t input_size,
	const size_t output_size,
	unsigned char* dict,
	size_t* dict_pos)
{
	assert(input);
	assert(dict);
	assert(dict_pos);

	unsigned char* output = new unsigned char[output_size];
	if (!output)
	{
		return NULL;
	}

	unsigned char* output_ptr = output;
	const unsigned char* input_ptr = input;
	const unsigned char* input_end = input_ptr + input_size;
	const unsigned char* output_end = output_ptr + output_size;

	int flags = 0;
	while (input_ptr < input_end)
	{
		flags >>= 1;
		if ((flags & 0x100) != 0x100)
		{
			if (input_ptr >= input_end)
				return output;
			flags = *input_ptr++ | 0xFF00;
		}

		if ((flags & 1) == 1)
		{
			if (input_ptr >= input_end)
				return output;
			unsigned char x0 = *input_ptr++;
			if (input_ptr >= input_end)
				return output;
			unsigned char x1 = *input_ptr++;
			size_t lookbehind_pos = x0 | ((x1 & 0xF) << 8);
			size_t lookbehind_size = 3 + ((x1 & 0xF0) >> 4);
			if (lookbehind_size == 18)
			{
				if (input_ptr >= input_end)
					return output;
				lookbehind_size += *input_ptr++;
			}

			for (size_t j = 0; j < lookbehind_size; j++)
			{
				unsigned char c = dict[lookbehind_pos];
				if (output_ptr >= output_end)
					return output;
				*output_ptr++ = c;
				dict[*dict_pos] = c;
				(*dict_pos)++;
				(*dict_pos) &= 0xFFF;
				lookbehind_pos++;
				lookbehind_pos &= 0xFFF;
			}
		}
		else
		{
			if (input_ptr >= input_end)
				return output;
			unsigned char c = *input_ptr++;
			if (output_ptr >= output_end)
				return output;
			*output_ptr++ = c;
			dict[*dict_pos] = c;
			(*dict_pos)++;
			(*dict_pos) &= 0xFFF;
		}
	}

	return output;
}