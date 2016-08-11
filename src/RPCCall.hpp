#ifndef _RPC_RPCCall
#define _RPC_RPCCall

#include <json.hpp>
#include "Exstension.hpp"

using json = nlohmann::json;

class RPCCall {
  friend class RPCCallNatives;
private:
  std::string method;
  IPluginFunction *callback;
  json *args;
public:
  RPCCall(IPluginFunction *callback);
  void SetMethod(std::string method);
  void SetArgsJSON(json *j);
  void Send(std::string server);
  void Broadcast();
  bool notification = false;
};


#endif //_RPC_RPCCall
