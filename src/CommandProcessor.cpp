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

  req["sig"] = this->getReqSig(&req);

  server->Send(req.dump());
  return RPCReqResult_Sent;
}

void CommandProcessor::HandleReply(std::string body) {
  try {
    auto j = json::parse(body);
    if(j.is_array()) {
      for(auto i = j.begin(); i != j.end(); i++) {
        auto reply = *i;
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
    } else if(j.is_object()) {
      if(!j["id"].is_null() && j["id"].is_string()) {
        std::string id = j["id"];
        try {
          auto call = this->outstandingCalls.at(id);
          call->HandleReply(&j);
        } catch(std::out_of_range e) {
          smutils->LogError(myself, "Got reply, but there is no record for request id %s", id.c_str());
        }
      }
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
      if (j["sig"].is_null()) {
        smutils->LogError(myself, "Bad signature for request: null");
        return cb(resp_invalid_request);
      } else {
        std::string sig = j["sig"];
        auto digest = this->getReqSig(&j);
        if (!(sig == digest)) {
          smutils->LogError(myself, "Bad signature for request: got %s, expected: %s", sig.c_str(), digest.c_str());
          return cb(resp_invalid_request);
        }
      }

      if (j["timestamp"].is_null() || !j["timestamp"].is_number_integer()) {
        smutils->LogError(myself, "Bad timestamp for request");
        return cb(resp_invalid_request);
      } else if (j["timestamp"].get<int>() < (time(nullptr) - 30)) {
        smutils->LogError(myself, "Request is too old");
        return cb(resp_invalid_request);
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
      method->Call(j["params"], [cb, id, notification](json retval) {
        if (!notification) {
          json resp;
          resp["jsonrpc"] = "2.0";
          resp["id"] = id;
          resp["result"] = retval;

          cb(std::make_shared<json>(resp));
        }
      });
    } else {
      cb(resp_invalid_request);
    }
  } catch (std::invalid_argument e) {
    cb(resp_parse_error);
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

std::string CommandProcessor::getReqSig(json *req) {
  std::stringstream ss;
  ss << req->at("jsonrpc").get<std::string>();
  ss << req->at("method").get<std::string>();
  ss << req->at("params").dump();
  ss << req->at("timestamp").get<int>();

  return Digest(ss.str(), this->apiKey);
}
