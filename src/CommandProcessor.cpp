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

#include "CommandProcessor.hpp"
#include "Crypto.hpp"
#include <ctime>
#include "rpc_handletypes.hpp"

CommandProcessor rpcCommandProcessor;

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

void CommandProcessor::RemoveServer(const std::string name) {
  this->servers.erase(name);
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
      auto call = this->outstandingCalls.at(id);
      call->HandleReply(reply);
      
      this->outstandingCalls.erase(id);
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

void CommandProcessor::HandleRequest(const std::string &remote, const std::string &req, request_callback cb) {
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
      method->Call(remote, j["params"], [cb, id, notification, this](json retval) {
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