#include "ByteUtil.h"

uint32_t byte2uint32(unsigned char* source) {
	uint32_t res = 0;
	short i = 3;

	do {
		res <<= 8;
		res += (unsigned short)source[i] & 0xFF;
		--i;
	} while (i >= 0);
	return res;
}

uint16_t byte2uint16(unsigned char* source) {
	uint16_t res = 0;
	short i = 2;

	do {
		res <<= 8;
		res += (unsigned short)source[i] & 0xFF;
		--i;
	} while (i >= 0);
	return res;
}

uint64_t byte2uint64(unsigned char* source) {
	uint64_t res = 0;
	short i = 8;

	do {
		res <<= 8;
		res += (unsigned short)source[i] & 0xFF;
		--i;
	} while (i >= 0);
	return res;
}

char* uint32t2byte(uint32_t source) {
	char* res = new char[4];
	for (int i = 0; i < 4; ++i) {
		res[i] = (unsigned short)source & 0xFF;
		source >>= 8;
	}
	return res;
}

char* uint64t2byte(uint64_t source) {
	char* res = new char[8];
	for (int i = 0; i < 8; ++i) {
		res[i] = (unsigned short)source & 0xFF;
		source >>= 8;
	}
	return res;
}

char* uint16t2byte(uint16_t source) {
	char* res = new char[2];
	for (int i = 0; i < 2; ++i) {
		res[i] = (unsigned short)source & 0xFF;
		source >>= 8;
	}
	return res;
}
