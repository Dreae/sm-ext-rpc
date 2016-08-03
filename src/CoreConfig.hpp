#ifndef _RPC_CoreConfig
#define _RPC_CoreConfig
#include <unordered_map>
#include <memory>

class ServerConfig {
public:
  int port = -1;
  std::string address = "";
};

class CoreConfig {
public:
  int port = -1;
  std::string address = "";
  std::string secret = "";
  std::unordered_map<std::string, std::shared_ptr<ServerConfig>> servers;
  void Init();
};

extern CoreConfig config;

#endif // !_RPC_CoreConfig
