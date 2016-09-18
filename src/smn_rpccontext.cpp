#include "Exstension.hpp"
#include "RPCContext.hpp"
#include "rpc_handletypes.hpp"

#define READ_HANDLE(pContext, params) \
  Handle_t hndl = static_cast<Handle_t>(params[1]); \
  HandleSecurity sec; \
  RPCContext *context; \
  sec.pOwner = pContext->GetIdentity(); \
  sec.pIdentity = myself->GetIdentity(); \
  auto herr = handlesys->ReadHandle(hndl, g_RPCContextType, &sec, reinterpret_cast<void **>(&context)); \
  if (herr != HandleError_None) { \
    return pContext->ThrowNativeError("Invalid RPCContext handle %x (error %d)", hndl, herr); \
  }
 
HandleType_t g_RPCContextType;
extern const sp_nativeinfo_t smrpc_context_natives[];

class RPCContextNatives : public SMRPCBase, public IHandleTypeDispatch{
public:
  void OnExtLoad() {
    HandleAccess hacc;
    TypeAccess tacc;

    handlesys->InitAccessDefaults(&tacc, &hacc);
    tacc.ident = myself->GetIdentity();
    hacc.access[HandleAccess_Read] = HANDLE_RESTRICT_OWNER;

    g_RPCContextType = handlesys->CreateType("RPCContext", this, 0, &tacc, &hacc, myself->GetIdentity(), NULL);
    sharesys->AddNatives(myself, smrpc_context_natives);
  }

  void OnExtUnload() {
    handlesys->RemoveType(g_RPCContextType, myself->GetIdentity());
  }

  void OnHandleDestroy(HandleType_t type, void *object) {
    delete reinterpret_cast<RPCContext *>(object);
  }

  // Used by HandleSys for reporting during Dump() and when freeing handles
  // after detecting a memory leak
  bool GetHandleApproxSize(HandleType_t type, void *object, unsigned int *size) {
    RPCContext *context = reinterpret_cast<RPCContext *>(object);
    *size = sizeof(RPCContext) + sizeof(json) + (context->params.size() * sizeof(json));

    return true;
  }
};

static RPCContextNatives natives;

static cell_t native_GetParams(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);
  auto paramsCopy = new json(context->params);

  auto jsonHndle = handlesys->CreateHandle(g_JSONType, paramsCopy, pContext->GetIdentity(), myself->GetIdentity(), NULL);

  return jsonHndle;
}

static cell_t native_FinishCall(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  context->finish();

  handlesys->FreeHandle(hndl, &sec);

  return 0;
}

static cell_t native_SetReturnJSON(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  Handle_t hndl2 = static_cast<Handle_t>(params[3]); 
  HandleSecurity sec2;
  json *obj2;
  sec2.pOwner = pContext->GetIdentity();
  sec2.pIdentity = myself->GetIdentity();
  auto herr2 = handlesys->ReadHandle(hndl2, g_JSONType, &sec2, reinterpret_cast<void **>(&obj2));
  if (herr2 != HandleError_None) {
    return pContext->ThrowNativeError("Invalid JSON handle %x (error %d)", hndl2, herr2);
  }

  char *key;
  pContext->LocalToString(params[2], &key);
  context->SetReturnValue<json>(*obj2);
  return 1;
}

const sp_nativeinfo_t smrpc_context_natives[] = {
  {"RPCContext.GetParams", native_GetParams},
  {"RPCContext.SetReturn", native_SetReturnJSON},
  {"RPCContext.Done", native_FinishCall},
  {NULL, NULL}
};