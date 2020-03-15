#pragma once
#include <openssl/evp.h>
#include <openssl/aes.h>

struct evp_cipher_ctx_st
{
	const EVP_CIPHER* cipher;
	ENGINE* engine;	/* functional reference if 'cipher' is ENGINE-provided */
	int encrypt;		/* encrypt or decrypt */
	int buf_len;		/* number we have left */

	unsigned char  oiv[EVP_MAX_IV_LENGTH];	/* original iv */
	unsigned char  iv[EVP_MAX_IV_LENGTH];	/* working iv */
	unsigned char buf[EVP_MAX_BLOCK_LENGTH];/* saved partial block */
	int num;				/* used by cfb/ofb mode */

	void* app_data;		/* application stuff */
	int key_len;		/* May change for variable length cipher */
	unsigned long flags;	/* Various flags */
	void* cipher_data; /* per EVP data */
	int final_used;
	int block_mask;
	unsigned char final[EVP_MAX_BLOCK_LENGTH];/* possible final block */
} /* EVP_CIPHER_CTX */;

class AesUtil
{
public:
	static void decode(const unsigned char* key, const unsigned char* iv, const unsigned char* src, unsigned char* out, uint32_t size);
	static void encode(const unsigned char* key, const unsigned char* iv, const unsigned char* src, unsigned char* out, uint32_t size, uint32_t* res_size);
};
