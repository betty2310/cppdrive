#include "crypto.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

#include <memory>

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
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return false;

    // Generate a random IV
    std::vector<unsigned char> iv(EVP_CIPHER_iv_length(EVP_aes_256_cbc()));
    RAND_bytes(iv.data(), iv.size());

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                           reinterpret_cast<const unsigned char *>(aesKey.data()),
                           iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> buffer(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int length = 0;
    int ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, buffer.data(), &length,
                          reinterpret_cast<const unsigned char *>(plaintext.data()),
                          plaintext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    ciphertext_len += length;

    if (EVP_EncryptFinal_ex(ctx, buffer.data() + length, &length) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    ciphertext_len += length;

    // Prepend IV to the ciphertext
    ciphertext.assign(reinterpret_cast<char *>(iv.data()), iv.size());
    ciphertext.append(reinterpret_cast<char *>(buffer.data()), ciphertext_len);

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

bool decrypt_data(const std::string &aesKey, const std::string &ciphertext,
                  std::string &plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return false;

    // Extract IV from the beginning of the ciphertext
    std::vector<unsigned char> iv(EVP_CIPHER_iv_length(EVP_aes_256_cbc()));
    memcpy(iv.data(), ciphertext.data(), iv.size());

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                           reinterpret_cast<const unsigned char *>(aesKey.data()),
                           iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> buffer(ciphertext.size() - iv.size());
    int length = 0;
    int plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, buffer.data(), &length,
                          reinterpret_cast<const unsigned char *>(ciphertext.data() + iv.size()),
                          ciphertext.size() - iv.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    plaintext_len += length;

    if (EVP_DecryptFinal_ex(ctx, buffer.data() + length, &length) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    plaintext_len += length;

    plaintext.assign(reinterpret_cast<char *>(buffer.data()), plaintext_len);

    EVP_CIPHER_CTX_free(ctx);
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
