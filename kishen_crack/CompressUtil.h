#pragma once
#include <zlib.h>
#include <cstdint>
class CompressUtil
{
public:
	static unsigned char* uncmp_deflate_without_header(unsigned char* data, uint32_t data_len, uint32_t uncmp_len);
	static unsigned char* lzss_decompress(const unsigned char* input, const size_t input_size, const size_t output_size, unsigned char* dict, size_t* dict_pos);
	static unsigned char* cmp_deflate_without_header(unsigned char* data, uint32_t uncmp_len, uint32_t* result_len);
};

