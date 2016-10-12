/**
 * Copyright 2019 Dreae
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include "Crypto.hpp"

std::string Digest(std::string message, std::string key) {
  auto digest = HMAC(EVP_sha256(), key.c_str(), key.size(), reinterpret_cast<const unsigned char *>(message.c_str()), message.size(), NULL, NULL);
  char hash[128];
  for (int c = 0; c < 32; c++) {
    sprintf(&hash[c * 2], "%02x", digest[c]);
  }

  return std::string(hash);
}