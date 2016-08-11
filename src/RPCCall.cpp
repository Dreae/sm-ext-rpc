#include "RPCCall.hpp"

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

}

void RPCCall::Broadcast() {

}