#include "RPCCall.hpp"
#include "CommandProcessor.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "rpc_handletypes.hpp"

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

std::string RPCCall::GetId() {
  return this->id;
}

void RPCCall::SetHandle(Handle_t handle) {
  this->handle = handle;
}

void RPCCall::Send(std::string server) {
  this->id = boost::lexical_cast<std::string>(uuidGenerator());
  
  json req;
  req["jsonrpc"] = "2.0";
  req["id"] = this->id;
  req["method"] = this->method;
  req["params"] = *this->args;

  rpcCommandProcessor.SendRequest(server, req, this);
}

void RPCCall::Notify(std::string server) {
  json req;
  req["jsonrpc"] = "2.0";
  req["method"] = this->method;
  req["params"] = *this->args;

  // rpcCommandProcessor.SendNotification(server, req);
}

void RPCCall::HandleReply(json *res) {
  auto reply = (*res)["result"];
  auto hndl = handlesys->CreateHandle(g_JSONType, &reply, this->callback->GetParentContext()->GetIdentity(), myself->GetIdentity(), NULL);
  this->callback->PushCell(hndl);
  this->callback->Execute(nullptr);

  HandleSecurity sec;
  sec.pOwner = this->callback->GetParentContext()->GetIdentity();
  sec.pIdentity = myself->GetIdentity();
  handlesys->FreeHandle(this->handle, &sec);
}

void RPCCall::Broadcast() {

}