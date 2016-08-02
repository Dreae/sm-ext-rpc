#include "Exstension.hpp"
#include "RPCContext.hpp"

#define READ_HANDLE(pContext, params) \
  Handle_t hndl = static_cast<Handle_t>(params[1]); \
  HandleSecurity sec; \
  RPCContext *context; \
  sec.pOwner = pContext->GetIdentity(); \
  sec.pIdentity = myself->GetIdentity(); \
  auto herr = handlesys->ReadHandle(hndl, g_RPCContextType, &sec, reinterpret_cast<void **>(&context));

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

  void OnHandleDestroy(HandleType_t type, void *object) {
    free(object);
  }

  bool GetHandleApproxSize(HandleType_t type, void *object, unsigned int *size) {
    return false;
  }
};

static RPCContextNatives natives;

static cell_t native_GetParamInt(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params)

  if (herr != HandleError_None) {
    return pContext->ThrowNativeError("Invalid RPCContext handle %x (error %d)", hndl, herr);
  }
  
  try {
    return context->ReadParam<int>(params[2]);
  } catch (std::domain_error e) {
    return pContext->ThrowNativeError("Invalid conversion, parameter %d is not an int", params[2]);
  }
}

static cell_t native_FinishCall(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params)

  if (herr != HandleError_None) {
    return pContext->ThrowNativeError("Invalid RPCContext handle %x (error %d)", hndl, herr);
  }
  context->callback("42"_json);

  handlesys->FreeHandle(hndl, &sec);

  return 0;
}


const sp_nativeinfo_t smrpc_context_natives[] = {
  {"RPCContext.GetParamInt", native_GetParamInt},
  {"RPCContext.Done", native_FinishCall},
  {NULL, NULL}
};