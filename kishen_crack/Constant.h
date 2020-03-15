#pragma once
#include <cstdint>

const unsigned char key[32] = {
0xBE,0x28,0x02,0xAD,0x5E,0x91,0xDD,0x8E,0x26,0xEA,0xD6,0xB1,0x61,0xFE,0xDB,0x8A,
0x17,0xE2,0x36,0x2F,0x53,0x33,0x6D,0x1B,0x17,0xD8,0x0A,0xE9,0x55,0xC0,0x5A,0xED
};

const int SEGMENT_SIZE = 0x10010; //分片的最长长度，由于aes的补位
const int MAX_SEGMENT_FILE_SIZE = 0x10000; //文件最大哦的分片大小

enum Result
{
	OK,
	COMPRESS_FAIL,
	OPEN_FILE_FAIL,
	CREATE_DICT_FAIL,
	UNPACK_ERROR_SIZE
};

typedef struct segment {
	uint64_t offset;//文件在封包的偏移
	uint32_t seg_size;//区段在封包的大小
	uint32_t seg_ualg_size;//区段未经对齐的大小
	uint32_t seg_ucmp_size;//区段未经压缩的大小
}Segment;

typedef struct file
{
	uint16_t name_len;
	char* name;
	char check[32];//校验值32字节
	uint32_t seg_count;//该文件包含多少个区段
	uint32_t uncomp_size;//文件未压缩之前的大小
	Segment* segments;
} File;