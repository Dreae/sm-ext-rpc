#ifndef _RPC_CommandProcessor
#define _RPC_CommandProcessor
#include <json.hpp>
#include <unordered_map>
#include "RPCMethod.hpp"
#include "Server.hpp"
#include "RPCCall.hpp"

static const std::shared_ptr<json> resp_parse_error = std::make_shared<json>("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32700, \"message\": \"Parse error\"}, \"id\": null}"_json);
static const std::shared_ptr<json> resp_invalid_request = std::make_shared<json>("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32600, \"message\": \"Invalid Request\"}, \"id\": null}"_json);

typedef std::function<void(std::shared_ptr<json>)> request_callback;

class CommandProcessor {
private:
  std::string apiKey;
  std::unordered_map<std::string, std::shared_ptr<RPCMethod>> methods;
  std::unordered_map<std::string, std::shared_ptr<Server>> servers;
  std::unordered_map<std::string, RPCCall *> outstandingCalls;

  std::shared_ptr<json> respError(int code, std::string message, json id);
  std::string getReqSig(json &req);
  std::string getReplySig(json &reply);
  void processCommandReply(json &reply);
public:
  void Init(std::string apiKey);
  void RegisterRPCMethod(std::string name, std::shared_ptr<RPCMethod> method);
  void HandleRequest(std::string body, request_callback callback);
  void HandleReply(std::string body);
  void RegisterServer(std::string name, std::shared_ptr<Server> server);
  RPCReqResult SendRequest(std::string target, json &req, RPCCall *call);
  void SendBroadcast(json &req);
};

extern CommandProcessor rpcCommandProcessor;

#endif // !_RPC_CommandProcessor
