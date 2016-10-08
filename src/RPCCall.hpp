#ifndef _RPC_RPCCall
#define _RPC_RPCCall

#include <json.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "Exstension.hpp"

enum RPCReqResult {
  RPCReqResult_UnknownServer,
  RPCReqResult_Sent
};

using json = nlohmann::json;

class RPCCall {
  friend class RPCCallNatives;
private:
  std::string method;
  IPluginFunction *callback;
  std::shared_ptr<json> args;
  std::string id;
  Handle_t handle;
public:
  RPCCall(IPluginFunction *callback);
  void SetMethod(std::string method);
  std::string GetId();
  void SetHandle(Handle_t handle);
  void SetArgsJSON(json *j);
  void HandleReply(json *res);
  RPCReqResult Send(std::string server);
  RPCReqResult Notify(std::string server);
  void Broadcast();
};

extern boost::uuids::random_generator uuidGenerator;

#endif //_RPC_RPCCall
