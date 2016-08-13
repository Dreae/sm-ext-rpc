#include "CommandProcessor.hpp"

CommandProcessor rpcCommandProcessor;

void CommandProcessor::RegisterRPCMethod(std::string name, std::shared_ptr<RPCMethod> method) {
  this->methods[name] = method;
}

void CommandProcessor::RegisterServer(std::string name, std::shared_ptr<Server> server) {
  this->servers[name] = server;
}

RPCReqResult CommandProcessor::SendRequest(std::string target, json req) {
  auto server = this->servers[target];
  if(!server) {
    return RPCReqResult_UnknownServer;
  }

  server->Send(req.dump());
  return RPCReqResult_Sent;
}

void CommandProcessor::HandleReply(std::string body) {

}

void CommandProcessor::HandleRequest(std::string req, request_callback cb) {
  try {
    auto j = json::parse(req);
    if (j.is_array()) {

    } else if (j.is_object()) {
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