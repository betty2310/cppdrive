#include "crypto.h"

#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

#include <memory>

std::vector<unsigned char> base64_decode(const std::string &in) {
    std::vector<unsigned char> out;

    BIO *bio, *b64;

    int decodeLen = in.size();
    out.resize(decodeLen);

    bio = BIO_new_mem_buf(in.c_str(), -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);   // Do not use newlines to flush buffer
    decodeLen = BIO_read(bio, out.data(), in.size());
    out.resize(decodeLen);

    BIO_free_all(bio);

    return out;
}

bool generate_symmetric_key(std::string &sym_key) {
    std::vector<unsigned char> key(SYMMETRIC_KEY_SIZE);
    if (RAND_bytes(key.data(), SYMMETRIC_KEY_SIZE) != 1) {
        // RAND_bytes failed
        return false;
    }

    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    if (!b64)
        return false;

    bio = BIO_new(BIO_s_mem());
    if (!bio) {
        BIO_free_all(b64);
        return false;
    }

    bio = BIO_push(b64, bio);

    if (BIO_write(bio, key.data(), key.size()) <= 0) {
        BIO_free_all(bio);
        return false;
    }

    if (BIO_flush(bio) <= 0) {
        BIO_free_all(bio);
        return false;
    }

    BIO_get_mem_ptr(bio, &bufferPtr);
    sym_key.assign(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);

    return true;
}

bool encrypt_data(const std::string &aesKey, const std::string &plaintext,
                  std::string &ciphertext) {
    std::vector<unsigned char> key = base64_decode(aesKey);
    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0x00, AES_BLOCK_SIZE);   // Hardcoded IV (for example purposes)

    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    unsigned char *out_buf = (unsigned char *) malloc(plaintext.size() + AES_BLOCK_SIZE);

    if (!(ctx = EVP_CIPHER_CTX_new()))
        return false;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key.data(), iv))
        return false;

    if (1 != EVP_EncryptUpdate(ctx, out_buf, &len, (unsigned char *) plaintext.c_str(),
                               plaintext.size()))
        return false;
    ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, out_buf + len, &len))
        return false;
    ciphertext_len += len;

    ciphertext = std::string((char *) out_buf, ciphertext_len);

    EVP_CIPHER_CTX_free(ctx);
    free(out_buf);

    return true;
}

bool decrypt_data(const std::string &aesKey, const std::string &ciphertext,
                  std::string &plaintext) {
    std::vector<unsigned char> key = base64_decode(aesKey);
    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0x00, AES_BLOCK_SIZE);   // Hardcoded IV

    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    unsigned char *out_buf = (unsigned char *) malloc(ciphertext.size());

    if (!(ctx = EVP_CIPHER_CTX_new()))
        return false;

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key.data(), iv))
        return false;

    if (1 != EVP_DecryptUpdate(ctx, out_buf, &len, (unsigned char *) ciphertext.c_str(),
                               ciphertext.size()))
        return false;
    plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, out_buf + len, &len))
        return false;
    plaintext_len += len;

    plaintext = std::string((char *) out_buf, plaintext_len);

    EVP_CIPHER_CTX_free(ctx);
    free(out_buf);

    return true;
}

bool generate_key_pair(std::string &public_key, std::string &private_key) {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx)
        return false;

    if (EVP_PKEY_keygen_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY *pkey = NULL;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }
    EVP_PKEY_CTX_free(ctx);

    BIO *pub = BIO_new(BIO_s_mem());
    BIO *pri = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(pub, pkey);
    PEM_write_bio_PrivateKey(pri, pkey, NULL, NULL, 0, NULL, NULL);

    size_t pub_len = BIO_pending(pub);
    size_t pri_len = BIO_pending(pri);

    public_key.resize(pub_len);
    private_key.resize(pri_len);

    BIO_read(pub, &public_key[0], pub_len);
    BIO_read(pri, &private_key[0], pri_len);

    BIO_free(pub);
    BIO_free(pri);
    EVP_PKEY_free(pkey);

    return true;
}

bool encrypt_symmetric_key(const std::string &publicKeyPEM, const std::string &plaintext,
                           std::string &encrypted) {
    auto bio = BIO_new_mem_buf(publicKeyPEM.data(), -1);
    auto publicKey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!publicKey)
        return false;

    auto ctx = EVP_PKEY_CTX_new(publicKey, nullptr);
    EVP_PKEY_free(publicKey);

    if (!ctx)
        return false;

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    size_t outlen;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen,
                         reinterpret_cast<const unsigned char *>(plaintext.data()),
                         plaintext.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> outbuf(outlen);
    if (EVP_PKEY_encrypt(ctx, outbuf.data(), &outlen,
                         reinterpret_cast<const unsigned char *>(plaintext.data()),
                         plaintext.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    encrypted.assign(reinterpret_cast<const char *>(outbuf.data()), outlen);
    EVP_PKEY_CTX_free(ctx);
    return true;
}

bool decrypt_symmetric_key(const std::string &privateKeyPEM, const std::string &encrypted,
                           std::string &decrypted) {
    auto bio = BIO_new_mem_buf(privateKeyPEM.data(), -1);
    auto privateKey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!privateKey)
        return false;

    auto ctx = EVP_PKEY_CTX_new(privateKey, nullptr);
    EVP_PKEY_free(privateKey);

    if (!ctx)
        return false;

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> encryptedData(encrypted.begin(), encrypted.end());

    size_t outlen;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, encryptedData.data(), encryptedData.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> outbuf(outlen);
    if (EVP_PKEY_decrypt(ctx, outbuf.data(), &outlen, encryptedData.data(), encryptedData.size()) <=
        0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    decrypted.assign(reinterpret_cast<char *>(outbuf.data()), outlen);
    EVP_PKEY_CTX_free(ctx);
    return true;
}
