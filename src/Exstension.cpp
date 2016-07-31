#include "Exstension.hpp"
#include "RPCMethod.hpp"
#include "EventLoop.hpp"
#include <thread>

Extension extension;
SMEXT_LINK(&extension);

void GameFrame(bool simulating) {
  eventLoop.Run();
}

extern const sp_nativeinfo_t smrpc_natives[];

bool Extension::SDK_OnLoad(char *error, size_t err_max, bool late) {
  smutils->AddGameFrameHook(&GameFrame);
  sharesys->AddNatives(myself, smrpc_natives);
  eventLoop.Init("butts", 1337);
  return true;
}

// native RPCRegisterMethod(char name[], RPCCallback function, ParameterType returnType, ParameterType ...)
cell_t RPCRegisterMethod(IPluginContext *pContext, const cell_t *params) {
  auto callback = pContext->GetFunctionById(params[2]);
  if (!callback) {
    pContext->ThrowNativeError("Invalid RPC callback specified");
  }

  auto paramCount = params[0];

  char *methodName;
  pContext->LocalToString(params[1], &methodName);

  auto returnType = static_cast<ParamType>(params[3]);

  auto paramTypes = std::unique_ptr<std::vector<ParamType>>(new std::vector<ParamType>());
  for (int c = 4; c < paramCount; c++) {
    cell_t *paramType;
    pContext->LocalToPhysAddr(params[c], &paramType);
    paramTypes->push_back(static_cast<ParamType>(*paramType));
  }
  auto method = std::unique_ptr<RPCMethod>(new RPCMethod(methodName, callback, returnType, std::move(paramTypes)));

  return false;
}

const sp_nativeinfo_t smrpc_natives[] = {
  {"RPCRegisterMethod", RPCRegisterMethod},
  {NULL, NULL}
};
