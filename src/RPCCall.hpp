#ifndef _RPC_RPCCall
#define _RPC_RPCCall

#include <json.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "Exstension.hpp"

using json = nlohmann::json;

class RPCCall {
  friend class RPCCallNatives;
private:
  std::string method;
  IPluginFunction *callback;
  json *args;
  std::string id;
public:
  RPCCall(IPluginFunction *callback);
  void SetMethod(std::string method);
  void SetArgsJSON(json *j);
  void Send(std::string server);
  void Notify(std::string server);
  void Broadcast();
};

extern boost::uuids::random_generator uuidGenerator;

#endif //_RPC_RPCCall
