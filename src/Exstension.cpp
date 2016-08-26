#include "Exstension.hpp"
#include "RPCMethod.hpp"
#include "EventLoop.hpp"
#include "CommandProcessor.hpp"
#include "CoreConfig.hpp"
#include "Server.hpp"
#include <thread>

Extension extension;
SMEXT_LINK(&extension);

SMRPCBase *SMRPCBase::head = NULL;
extern const sp_nativeinfo_t smrpc_natives[];

bool Extension::SDK_OnLoad(char *error, size_t err_max, bool late) {
  sharesys->AddNatives(myself, smrpc_natives);

  config.Init();

  if (config.port <= 0 || config.port > 65535) {
    strcpy(error, "Invalid listen port provided in configuration");
    return false;
  } else if (config.address == "") {
    strcpy(error, "No listen address provided in configuration");
    return false;
  } else if (config.secret == "") {
    strcpy(error, "No signing key provided in configuration");
    return false;
  } else {
    eventLoop.Init(config.port);
    rpcCommandProcessor.Init(config.secret);

    for (auto n : config.servers) {
      auto server = std::make_shared<Server>(n.second->address, n.second->port);
      server->Connect(nullptr);
      eventLoop.RegisterService(server);
      rpcCommandProcessor.RegisterServer(n.first, server);
    }


    SMRPCBase *head = SMRPCBase::head;
    while (head) {
      head->OnExtLoad();
      head = head->next;
    }

    return true;
  }
}

void Extension::SDK_OnUnload() {
  SMRPCBase *head = SMRPCBase::head;
  while (head) {
    head->OnExtUnload();
    head = head->next;
  }
}

// native void RPCRegisterMethod(char[] name, RPCCallback callback, ParameterType ...);
cell_t RPCRegisterMethod(IPluginContext *pContext, const cell_t *params) {
  auto callback = pContext->GetFunctionById((funcid_t)params[2]);
  if (!callback) {
    pContext->ThrowNativeError("Invalid RPC callback specified");
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

const sp_nativeinfo_t smrpc_natives[] = {
  {"RPCRegisterMethod", RPCRegisterMethod},
  {NULL, NULL}
};
