#include "RPCContext.hpp"

RPCContext::RPCContext(json params, std::function<void(json)> callback) {
  this->params = params;
  this->callback = callback;
}

void RPCContext::finish() {
  this->callback(this->retval);
}