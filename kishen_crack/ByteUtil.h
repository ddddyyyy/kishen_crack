#ifndef BYTE_H
#define BYTE_H
#include <cstdint>

uint32_t byte2uint32(unsigned char* source);
uint16_t byte2uint16(unsigned char* source);
uint64_t byte2uint64(unsigned char* source);

char* uint32t2byte(uint32_t source);
char* uint64t2byte(uint64_t source);
char* uint16t2byte(uint16_t source);

#endif // !BYTE_H
