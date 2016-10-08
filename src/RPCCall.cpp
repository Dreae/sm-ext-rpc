#include "RPCCall.hpp"
#include "CommandProcessor.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "rpc_handletypes.hpp"

boost::uuids::random_generator uuidGenerator;

RPCCall::RPCCall(IPluginFunction *callback) {
  this->callback = callback;
  this->args = std::make_shared<json>();
}

void RPCCall::SetMethod(std::string method) {
  this->method = method;
}

void RPCCall::SetArgsJSON(json *j) {
  this->args = std::make_shared<json>(*j);
}

std::string RPCCall::GetId() {
  return this->id;
}

void RPCCall::SetHandle(Handle_t handle) {
  this->handle = handle;
}

RPCReqResult RPCCall::Send(std::string server) {
  this->id = boost::lexical_cast<std::string>(uuidGenerator());
  
  json req;
  req["jsonrpc"] = "2.0";
  req["id"] = this->id;
  req["method"] = this->method;
  if (this->args != nullptr) {
    req["params"] = *this->args;
  }

  return rpcCommandProcessor.SendRequest(server, req, this);
}

RPCReqResult RPCCall::Notify(std::string server) {
  json req;
  req["jsonrpc"] = "2.0";
  req["method"] = this->method;
  if (this->args != nullptr) {
    req["params"] = *this->args;
  }

  return rpcCommandProcessor.SendRequest(server, req, nullptr);
}

void RPCCall::HandleReply(json *res) {
  json *reply = new json((*res)["result"]);
  auto hndl = handlesys->CreateHandle(g_JSONType, reply, this->callback->GetParentContext()->GetIdentity(), myself->GetIdentity(), NULL);
  this->callback->PushCell(hndl);
  this->callback->Execute(nullptr);

  HandleSecurity sec;
  sec.pOwner = this->callback->GetParentContext()->GetIdentity();
  sec.pIdentity = myself->GetIdentity();
  handlesys->FreeHandle(this->handle, &sec);
}

void RPCCall::Broadcast() {
  json req;
  req["jsonrpc"] = "2.0";
  req["method"] = this->method;
  if (this->args != nullptr) {
    req["params"] = *this->args;
  }

  rpcCommandProcessor.SendBroadcast(req);
}