#include "RPCCall.hpp"
#include "CommandProcessor.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>

boost::uuids::random_generator uuidGenerator;

RPCCall::RPCCall(IPluginFunction *callback) {
  this->callback = callback;
}

void RPCCall::SetMethod(std::string method) {
  this->method = method;
}

void RPCCall::SetArgsJSON(json *j) {
  this->args = j;
}

void RPCCall::Send(std::string server) {
  this->id = boost::lexical_cast<std::string>(uuidGenerator());
  json req;
  req["jsonrpc"] = "2.0";
  req["id"] = this->id;
  req["method"] = this->method;
  req["params"] = *this->args;

  rpcCommandProcessor.SendRequest(server, req);
}

void RPCCall::Notify(std::string server) {
  json req;
  req["jsonrpc"] = "2.0";
  req["method"] = this->method;
  req["params"] = *this->args;

  rpcCommandProcessor.SendRequest(server, req);
}

void RPCCall::Broadcast() {

}