#include "Exstension.hpp"
#include "rpc_handletypes.hpp"
#include "CommandProcessor.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>

extern const sp_nativeinfo_t smrpc_natives[];

class RPCNatives : public SMRPCBase {
public:
  void OnExtLoad() {
    sharesys->AddNatives(myself, smrpc_natives);
  }
};

RPCNatives natives;

boost::uuids::random_generator uuidGenerator;

// native void RPCRegisterMethod(char[] name, RPCCallback callback, ParameterType ...);
static cell_t RPCRegisterMethod(IPluginContext *pContext, const cell_t *params) {
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
static cell_t RPCGetServers(IPluginContext *pContext, const cell_t *params) {
  auto server_list = rpcCommandProcessor.GetServers();
  auto servers = new json;
  for (auto it = server_list.begin(); it != server_list.end(); ++it) {
    servers->push_back(it->first);
  }

  auto hndl = handlesys->CreateHandle(g_JSONType, servers, pContext->GetIdentity(), myself->GetIdentity(), NULL);

  return hndl;
}

// native void RPCAddServer(const char[] server, const char[] address, int port);
static cell_t RPCAddServer(IPluginContext *pContext, const cell_t *params) {
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

// native void RPCRemoveServer(const char[] server);
static cell_t RPCRemoveServer(IPluginContext *pContext, const cell_t *params) {
  char *name;
  pContext->LocalToString(params[1], &name);

  auto server_str = std::string(name);
  auto current_server = rpcCommandProcessor.GetServer(server_str);
  if(!current_server) {
    pContext->ReportError("Server %s does not exist", name);
    return 0;
  }

  rpcCommandProcessor.RemoveServer(server_str);
  return 1;
}

// native int RPCGetServerPort(const char[] server);
static cell_t RPCGetServerPort(IPluginContext *pContext, const cell_t *params) {
  char *name;
  pContext->LocalToString(params[1], &name);

  auto server_str = std::string(name);
  auto current_server = rpcCommandProcessor.GetServer(name);
  if(!current_server) {
    pContext->ReportError("Server %s does not exist", name);
    return 0;
  }

  return current_server->GetPort();
}

// native void RPCGetServerAddress(const char[] server, char[] addres, int maxSize);
static cell_t RPCGetServerAddress(IPluginContext *pContext, const cell_t *params) {
  char *name;
  pContext->LocalToString(params[1], &name);

  auto server_str = std::string(name);
  auto current_server = rpcCommandProcessor.GetServer(name);
  if(!current_server) {
    pContext->ReportError("Server %s does not exist", name);
    return 0;
  }

  pContext->StringToLocal(params[2], params[3], current_server->GetAddress().c_str());
  return 1;
}

// native void RPCGetUUID(char[] buffer, int maxSize);
static cell_t RPCGetUUID(IPluginContext *pContext, const cell_t *params) {
  auto uuid = boost::lexical_cast<std::string>(uuidGenerator());

  pContext->StringToLocal(params[1], params[2], uuid.c_str());
  return 1;
}

const sp_nativeinfo_t smrpc_natives[] = {
  {"RPCRegisterMethod", RPCRegisterMethod},
  {"RPCGetServers", RPCGetServers},
  {"RPCAddServer", RPCAddServer},
  {"RPCRemoveServer", RPCRemoveServer},
  {"RPCGetServerAddress", RPCGetServerAddress},
  {"RPCGetServerPort", RPCGetServerPort},
  {"RPCGetUUID", RPCGetUUID},
  {NULL, NULL}
};
