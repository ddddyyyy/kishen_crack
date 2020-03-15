#pragma once
#include <cstdint>

const unsigned char key[32] = {
0xBE,0x28,0x02,0xAD,0x5E,0x91,0xDD,0x8E,0x26,0xEA,0xD6,0xB1,0x61,0xFE,0xDB,0x8A,
0x17,0xE2,0x36,0x2F,0x53,0x33,0x6D,0x1B,0x17,0xD8,0x0A,0xE9,0x55,0xC0,0x5A,0xED
};

const int SEGMENT_SIZE = 0x10010; //��Ƭ������ȣ�����aes�Ĳ�λ
const int MAX_SEGMENT_FILE_SIZE = 0x10000; //�ļ����Ŷ�ķ�Ƭ��С

enum Result
{
	OK,
	COMPRESS_FAIL,
	OPEN_FILE_FAIL,
	CREATE_DICT_FAIL,
	UNPACK_ERROR_SIZE
};

typedef struct segment {
	uint64_t offset;//�ļ��ڷ����ƫ��
	uint32_t seg_size;//�����ڷ���Ĵ�С
	uint32_t seg_ualg_size;//����δ������Ĵ�С
	uint32_t seg_ucmp_size;//����δ��ѹ���Ĵ�С
}Segment;

typedef struct file
{
	uint16_t name_len;
	char* name;
	char check[32];//У��ֵ32�ֽ�
	uint32_t seg_count;//���ļ��������ٸ�����
	uint32_t uncomp_size;//�ļ�δѹ��֮ǰ�Ĵ�С
	Segment* segments;
} File;