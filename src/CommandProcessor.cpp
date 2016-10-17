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

#include "CommandProcessor.hpp"
#include "Crypto.hpp"
#include <ctime>
#include "rpc_handletypes.hpp"

CommandProcessor rpcCommandProcessor;
extern const sp_nativeinfo_t smrpc_natives[];

void CommandProcessor::Init(std::string apiKey) {
  this->apiKey = apiKey;
}

void CommandProcessor::RegisterRPCMethod(const std::string name, std::shared_ptr<RPCMethod> method) {
  this->methods[name] = method;
}

void CommandProcessor::RegisterServer(const std::string name, std::shared_ptr<Server> server) {
  this->servers[name] = server;
}

std::shared_ptr<Server> CommandProcessor::GetServer(const std::string name) {
  return this->servers[name];
}

RPCReqResult CommandProcessor::SendRequest(const std::string &target, json &req, RPCCall *call) {
  auto server = this->servers[target];
  if(!server) {
    return RPCReqResult_UnknownServer;
  }

  if (call != nullptr) {
    this->outstandingCalls[call->GetId()] = call;
  }

  req["timestamp"] = time(nullptr);

  req["sig"] = this->getReqSig(req);

  server->Send(req.dump());
  return RPCReqResult_Sent;
}

void CommandProcessor::SendBroadcast(json &req) {
  req["timestamp"] = time(nullptr);
  req["sig"] = this->getReqSig(req);

  auto body = req.dump();
  for(auto it = this->servers.begin(); it != this->servers.end(); ++it) {
    it->second->Send(body);
  }
}

void CommandProcessor::processCommandReply(json &reply) {
  if(!reply["error"].is_null()) {
    smutils->LogError(myself, "Got error reply: %s", reply.dump().c_str());
    return;
  }

  if(!reply["sig"].is_string()) {
    smutils->LogError(myself, "Invalid reply signature");
    return;
  } else if(reply["sig"].get<std::string>() != this->getReplySig(reply)) {
    smutils->LogError(myself, "Got bad signature for reply");
    return;
  }

  if(!reply["id"].is_null() && reply["id"].is_string()) {
    std::string id = reply["id"];
    try {
      auto call = this->outstandingCalls[id];
      call->HandleReply(&reply);
    } catch(std::out_of_range e) {
      smutils->LogError(myself, "Got reply, but there is no record for request id %s", id.c_str());
    }
  }
}

void CommandProcessor::HandleReply(const std::string &body) {
  try {
    auto j = json::parse(body);
    if(j.is_array()) {
      for(auto i = j.begin(); i != j.end(); i++) {
        auto reply = *i;
        processCommandReply(reply);
      }
    } else if(j.is_object()) {
      processCommandReply(j);
    }
  } catch(std::invalid_argument e) {
    smutils->LogError(myself, "Error parsing RPC reply: %s", e.what());
  }
}

void CommandProcessor::HandleRequest(const std::string &req, request_callback cb) {
  try {
    auto j = json::parse(req);
    if (j.is_array()) {

    } else if (j.is_object()) {
      if (j["timestamp"].is_null()) {
        smutils->LogError(myself, "Bad timestamp for request");
        return cb(resp_invalid_request);
      } else if (!j["timestamp"].is_number_integer()) {
        smutils->LogError(myself, "Request timestamp is not a number");
        return cb(resp_invalid_request); 
      } else if (j["timestamp"].get<int>() < (time(nullptr) - 30)) {
        smutils->LogError(myself, "Request is too old");
        return cb(resp_invalid_request);
      }

      if (j["sig"].is_null()) {
        smutils->LogError(myself, "Bad signature for request: null");
        return cb(resp_invalid_request);
      } else {
        std::string sig = j["sig"];
        auto digest = this->getReqSig(j);
        if (!(sig == digest)) {
          smutils->LogError(myself, "Bad signature for request: got %s, expected: %s", sig.c_str(), digest.c_str());
          return cb(resp_invalid_request);
        }
      }

      if (!j["params"].is_array() || !j["method"].is_string()) {
        return cb(resp_invalid_request);
      }

      auto notification = !(j["id"].is_number() || j["id"].is_string());

      std::string methodName = j["method"];
      auto method = this->methods[methodName];
      if (!method) {
        if (!notification) {
          cb(this->respError(-32601, "Method not found", j["id"]));
        }

        return;
      }

      if (!method->ValidateArguments(j["params"])) {
        if (!notification) {
          cb(this->respError(-32602, "Invalid params", j["id"]));
        }

        return;
      }

      auto id = j["id"];
      method->Call(j["params"], [cb, id, notification, this](json retval) {
        if (!notification) {
          json resp;
          resp["jsonrpc"] = "2.0";
          resp["id"] = id;
          resp["result"] = retval;
          resp["sig"] = this->getReplySig(resp);

          cb(std::make_shared<json>(resp));
        }
      });
    } else {
      cb(resp_invalid_request);
    }
  } catch (std::invalid_argument) {
    cb(resp_parse_error);
  } catch (std::exception &ex) {
    smutils->LogError(myself, "Error handling request: %s", ex.what());
    cb(resp_invalid_request);
  }
}

std::shared_ptr<json> CommandProcessor::respError(int code, const std::string &message, json id) {
  json resp;
  resp["jsonrpc"] = "2.0";
  resp["error"]["code"] = code;
  resp["error"]["message"] = message;
  resp["id"] = id;

  return std::make_shared<json>(resp);
}

std::string CommandProcessor::getReqSig(json &req) {
  std::stringstream ss;
  ss << req["jsonrpc"].dump();
  ss << req["method"].dump();
  ss << req["params"].dump();
  ss << req["timestamp"].dump();
  ss << req["id"].dump();

  return Digest(ss.str(), this->apiKey);
}

std::string CommandProcessor::getReplySig(json &reply) {
  std::stringstream ss;
  ss << reply["jsonrpc"].dump();
  ss << reply["id"].dump();
  ss << reply["result"].dump();

  return Digest(ss.str(), this->apiKey);
}

const std::unordered_map<std::string, std::shared_ptr<Server>>& CommandProcessor::GetServers() {
  return this->servers;
}

void CommandProcessor::OnExtLoad() {
  sharesys->AddNatives(myself, smrpc_natives);
}

// native void RPCRegisterMethod(char[] name, RPCCallback callback, ParameterType ...);
cell_t RPCRegisterMethod(IPluginContext *pContext, const cell_t *params) {
  auto callback = pContext->GetFunctionById((funcid_t)params[2]);
  if (!callback) {
    pContext->ReportError("Invalid RPC callback specified");
  }

  auto paramCount = params[0];

  char *methodName;
  pContext->LocalToString(params[1], &methodName);

  auto paramTypes = std::unique_ptr<std::vector<ParamType>>(new std::vector<ParamType>());
  for (int c = 3; c < paramCount + 1; c++) {
    cell_t *paramType;
    pContext->LocalToPhysAddr(params[c], &paramType);
    paramTypes->push_back(static_cast<ParamType>(*paramType));
  }
  auto method = std::make_shared<RPCMethod>(methodName, pContext, callback, std::move(paramTypes));
  rpcCommandProcessor.RegisterRPCMethod(methodName, method);

  return false;
}

// native void RPCGetServers();
cell_t RPCGetServers(IPluginContext *pContext, const cell_t *params) {
  auto server_list = rpcCommandProcessor.GetServers();
  auto servers = new json;
  for (auto it = server_list.begin(); it != server_list.end(); ++it) {
    servers->push_back(it->first);
  }

  auto hndl = handlesys->CreateHandle(g_JSONType, servers, pContext->GetIdentity(), myself->GetIdentity(), NULL);

  return hndl;
}

// native void RPCAddServer(const char server[], const char address[], int port);
cell_t RPCAddServer(IPluginContext *pContext, const cell_t *params) {
  char *server_name;
  pContext->LocalToString(params[1], &server_name);

  char *server_address;
  pContext->LocalToString(params[2], &server_address);

  auto port = params[3];

  auto server_str = std::string(server_name);
  auto current_server = rpcCommandProcessor.GetServer(server_str);
  if(current_server) {
    pContext->ReportError("Server %s already exists", server_name);
    return 0;
  }

  auto server = std::make_shared<Server>(server_str, port);

  server->Connect(nullptr);
  eventLoop.RegisterService(server);
  rpcCommandProcessor.RegisterServer(server_str, server);

  return 1;
}

const sp_nativeinfo_t smrpc_natives[] = {
  {"RPCRegisterMethod", RPCRegisterMethod},
  {"RPCGetServers", RPCGetServers},
  {"RPCAddServer", RPCAddServer},
  {NULL, NULL}
};
