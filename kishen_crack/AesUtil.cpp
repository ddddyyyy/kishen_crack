#include "AesUtil.h"


void AesUtil::decode(const unsigned char* key, const unsigned char* iv, const unsigned char* src, unsigned char* out, uint32_t size)
{
	AES_KEY aes_key;
	AES_set_decrypt_key(key, 256, &aes_key);
	unsigned char temp[16];
	for (int i = 0; i < 16; ++i) {
		temp[i] = iv[i];
	}
	AES_cbc_encrypt(src, out, size, &aes_key, temp, AES_DECRYPT);
}
evp_cipher_ctx_st ctx;

void AesUtil::encode(const unsigned char* key, const unsigned char* iv, const unsigned char* src, unsigned char* out, uint32_t size, uint32_t* res_size) {

	EVP_EncryptInit(
		&ctx,
		EVP_aes_256_cbc(),
		key,
		iv);
	int actual_size = 0;
	EVP_EncryptUpdate(
		&ctx,
		out,
		&actual_size,
		src,
		size);

	int final_size = 0;
	EVP_EncryptFinal(&ctx, out + actual_size, &final_size);
	actual_size += final_size;
	*res_size = actual_size;
}