#ifndef __CYPHER_H__
#define __CYPHER_H__

#include <string>
#include <vector>

/**
 * Generate RSA key pair
 */
bool generate_key_pair(std::string &public_key, std::string &private_key);

bool encrypt_data(const std::string &publicKeyPEM, const std::string &plaintext,
                  std::vector<unsigned char> &encrypted);

bool decrypt_data(const std::string &privateKeyPEM, const std::vector<unsigned char> &encrypted,
                  std::string &decrypted);

#endif   // !__CYPHER_H__
