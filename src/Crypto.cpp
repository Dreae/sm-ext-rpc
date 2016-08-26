#include "Crypto.hpp"

std::string Digest(std::string message, std::string key) {
  auto digest = HMAC(EVP_sha256(), key.c_str(), key.size(), reinterpret_cast<const unsigned char *>(message.c_str()), message.size(), NULL, NULL);
  char hash[64];
  for (int c = 0; c < 32; c++) {
    sprintf(&hash[c * 2], "%02x", digest[c]);
  }

  return std::string(hash);
}