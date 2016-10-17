/**
 * Copyright 2016 Will Austin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include "Exstension.hpp"
#include "RPCCall.hpp"
#include "rpc_handletypes.hpp"

#define READ_HANDLE(pContext, params) \
  Handle_t hndl = static_cast<Handle_t>(params[1]); \
  HandleSecurity sec; \
  RPCCall *obj; \
  sec.pOwner = pContext->GetIdentity(); \
  sec.pIdentity = myself->GetIdentity(); \
  auto herr = handlesys->ReadHandle(hndl, g_RPCCallType, &sec, reinterpret_cast<void **>(&obj)); \
  if (herr != HandleError_None) { \
    pContext->ReportError("Invalid RPCCall handle %x (error %d)", hndl, herr); \
    return 0; \
  }

HandleType_t g_RPCCallType;
extern const sp_nativeinfo_t rpc_call_natives[];

class RPCCallNatives : public SMRPCBase, public IHandleTypeDispatch {
public:
  void OnExtLoad() {
    HandleAccess hacc;
    TypeAccess tacc;

    handlesys->InitAccessDefaults(&tacc, &hacc);
    tacc.ident = myself->GetIdentity();
    hacc.access[HandleAccess_Read] = HANDLE_RESTRICT_OWNER;
    tacc.access[HTypeAccess_Create] = true;
    tacc.access[HTypeAccess_Inherit] = true;

    g_RPCCallType = handlesys->CreateType("RPCCall", this, 0, &tacc, &hacc, myself->GetIdentity(), NULL);
    sharesys->AddNatives(myself, rpc_call_natives);
  }

  void OnExtUnload() {
    handlesys->RemoveType(g_RPCCallType, myself->GetIdentity());
  }

  void OnHandleDestroy(HandleType_t type, void *object) {
    delete reinterpret_cast<RPCCall *>(object);
  }

  // Used by HandleSys for reporting during Dump() and when freeing handles
  // after detecting a memory leak
  bool GetHandleApproxSize(HandleType_t type, void *object, unsigned int *size) {
    RPCCall *obj = reinterpret_cast<RPCCall *>(object);
    *size = sizeof(RPCCall) + (sizeof(json) * obj->args->size());

    return true;
  }
};

static RPCCallNatives natives;

static cell_t native_CreateRPCCall(IPluginContext *pContext, const cell_t *params) {
  auto rpcCall = new RPCCall(nullptr, pContext->GetIdentity());
  auto hndl = handlesys->CreateHandle(g_RPCCallType, rpcCall, pContext->GetIdentity(), myself->GetIdentity(), NULL);

  rpcCall->SetHandle(hndl);
  return hndl;
}

static cell_t native_RPCCallSetMethod(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *method;
  pContext->LocalToString(params[2], &method);

  obj->SetMethod(std::string(method));
  return 1;
}

static cell_t native_RPCCallSend(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  auto callback = pContext->GetFunctionById((funcid_t)params[3]);
  if (!callback) {
    pContext->ReportError("Invalid RPC callback specified");
    return 0;
  }

  obj->SetCallback(callback);

  char *server;
  pContext->LocalToString(params[2], &server);

  auto res = obj->Send(std::string(server)); 
  
  if (res == RPCReqResult_UnknownServer) {
    pContext->ReportError("Error: Unknown Server %s", server);
    return 0;
  } else {
    return 1;
  }
}

static cell_t native_RPCCallNotify(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *server;
  pContext->LocalToString(params[2], &server);

  auto res = obj->Notify(std::string(server));
  
  if (res == RPCReqResult_UnknownServer) {
    pContext->ReportError("Error: Unknown Server %s", server);
    return 0;
  } else {
    return 1;
  }
}

static cell_t native_RPCCallBroadcast(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  obj->Broadcast();
  return 1;
}

static cell_t native_RPCCallSetParamsJSON(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  auto jsonHandle = static_cast<Handle_t>(params[2]);
  HandleSecurity jsonSec;
  jsonSec.pOwner = pContext->GetIdentity();
  jsonSec.pIdentity = myself->GetIdentity();
  json *j;
  auto jsonErr = handlesys->ReadHandle(jsonHandle, g_JSONType, &jsonSec, reinterpret_cast<void **>(&j));
  if(jsonErr != HandleError_None) {
    pContext->ReportError("Invalid JSON handle %x (error %d)", jsonHandle, jsonErr);
    return 0;
  }

  obj->SetArgsJSON(j);
  return 1;
}

const sp_nativeinfo_t rpc_call_natives[] = {
  {"RPCCall.RPCCall", native_CreateRPCCall},
  {"RPCCall.SetMethod", native_RPCCallSetMethod},
  {"RPCCall.SetParams", native_RPCCallSetParamsJSON},
  {"RPCCall.Send", native_RPCCallSend},
  {"RPCCall.Broadcast", native_RPCCallBroadcast},
  {"RPCCall.Notify", native_RPCCallNotify},
  {NULL, NULL}
};