#include "crypto.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <memory>

std::unique_ptr<BIO, decltype(&BIO_free)> create_bio() {
    return std::unique_ptr<BIO, decltype(&BIO_free)>(BIO_new(BIO_s_mem()), BIO_free);
}

std::string bio_str(BIO *bio) {
    size_t length = BIO_pending(bio);
    std::string str(length, '\0');
    BIO_read(bio, &str[0], length);
    return str;
}

bool generate_key_pair(std::string &public_key, std::string &private_key) {
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pkey_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

    if (EVP_PKEY_keygen_init(pkey_ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_keygen_bits(pkey_ctx, 2048) <= 0 ||
        EVP_PKEY_keygen(pkey_ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(pkey_ctx);
        return false;
    }

    EVP_PKEY_CTX_free(pkey_ctx);

    auto pubBio = create_bio();
    auto priBio = create_bio();

    if (!PEM_write_bio_PUBKEY(pubBio.get(), pkey) ||
        !PEM_write_bio_PrivateKey(priBio.get(), pkey, NULL, NULL, 0, NULL, NULL)) {
        EVP_PKEY_free(pkey);
        return false;
    }

    public_key = bio_str(pubBio.get());
    private_key = bio_str(priBio.get());

    EVP_PKEY_free(pkey);
    return true;
}

bool encrypt_data(const std::string &publicKeyPEM, const std::string &plaintext,
                  std::vector<unsigned char> &encrypted) {
    auto bio = BIO_new_mem_buf(publicKeyPEM.data(), -1);
    auto publicKey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!publicKey) {
        return false;
    }

    auto ctx = EVP_PKEY_CTX_new(publicKey, nullptr);
    EVP_PKEY_free(publicKey);

    if (!ctx) {
        return false;
    }

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

    encrypted.resize(outlen);
    if (EVP_PKEY_encrypt(ctx, encrypted.data(), &outlen,
                         reinterpret_cast<const unsigned char *>(plaintext.data()),
                         plaintext.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY_CTX_free(ctx);
    return true;
}

bool decrypt_data(const std::string &privateKeyPEM, const std::vector<unsigned char> &encrypted,
                  std::string &decrypted) {
    auto bio = BIO_new_mem_buf(privateKeyPEM.data(), -1);
    auto privateKey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!privateKey) {
        return false;
    }

    auto ctx = EVP_PKEY_CTX_new(privateKey, nullptr);
    EVP_PKEY_free(privateKey);

    if (!ctx) {
        return false;
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    size_t outlen;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, encrypted.data(), encrypted.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> outbuf(outlen);
    if (EVP_PKEY_decrypt(ctx, outbuf.data(), &outlen, encrypted.data(), encrypted.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    decrypted.assign(reinterpret_cast<char *>(outbuf.data()), outlen);
    EVP_PKEY_CTX_free(ctx);
    return true;
}
