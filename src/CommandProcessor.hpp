/**
 * Copyright 2016 Will Austin
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

class CommandProcessor : public SMRPCBase {
private:
  std::string apiKey;
  std::unordered_map<std::string, std::shared_ptr<RPCMethod>> methods;
  std::unordered_map<std::string, std::shared_ptr<Server>> servers;
  std::unordered_map<std::string, RPCCall *> outstandingCalls;

  std::shared_ptr<json> respError(int code, const std::string &message, json id);
  std::string getReqSig(json &req);
  std::string getReplySig(json &reply);
  void processCommandReply(json &reply);
public:
  void Init(std::string apiKey);
  void OnExtLoad();
  void RegisterRPCMethod(const std::string name, std::shared_ptr<RPCMethod> method);
  void HandleRequest(const std::string &body, request_callback callback);
  void HandleReply(const std::string &body);
  void RegisterServer(const std::string name, std::shared_ptr<Server> server);
  std::shared_ptr<Server> GetServer(const std::string name);
  const std::unordered_map<std::string, std::shared_ptr<Server>>& GetServers();
  RPCReqResult SendRequest(const std::string &target, json &req, RPCCall *call);
  void SendBroadcast(json &req);
};

extern CommandProcessor rpcCommandProcessor;

#endif // !_RPC_CommandProcessor
