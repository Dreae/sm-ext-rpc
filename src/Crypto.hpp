#ifndef _RPC_Crypto
#define _RPC_Crypto
#include <openssl/hmac.h>
#include <string>

std::string Digest(std::string message, std::string key);

#endif // !_RPC_Crypto
