#include "CommandProcessor.hpp"
#include "Crypto.hpp"
#include <ctime>

CommandProcessor rpcCommandProcessor;

void CommandProcessor::Init(std::string apiKey) {
  this->apiKey = apiKey;
}

void CommandProcessor::RegisterRPCMethod(std::string name, std::shared_ptr<RPCMethod> method) {
  this->methods[name] = method;
}

void CommandProcessor::RegisterServer(std::string name, std::shared_ptr<Server> server) {
  this->servers[name] = server;
}

RPCReqResult CommandProcessor::SendRequest(std::string target, json req, RPCCall *call) {
  auto server = this->servers[target];
  if(!server) {
    return RPCReqResult_UnknownServer;
  }

  this->outstandingCalls[call->GetId()] = call;
  req["timestamp"] = time(nullptr);

  req["sig"] = this->getReqSig(req);

  server->Send(req.dump());
  return RPCReqResult_Sent;
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

void CommandProcessor::HandleReply(std::string body) {
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

void CommandProcessor::HandleRequest(std::string req, request_callback cb) {
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
  } catch (std::invalid_argument &ex) {
    cb(resp_parse_error);
  } catch (std::exception &ex) {
    smutils->LogError(myself, "Error handling request: %s", ex.what());
    cb(resp_invalid_request);
  }
}

std::shared_ptr<json> CommandProcessor::respError(int code, std::string message, json id) {
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