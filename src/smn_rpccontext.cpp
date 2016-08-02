#include "Exstension.hpp"
#include "RPCContext.hpp"

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

static cell_t native_GetParamInt(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  try {
    return context->ReadParam<int>(params[2]);
  } catch (std::domain_error e) {
    return pContext->ThrowNativeError("Invalid conversion, parameter %d is not an int", params[2]);
  }
}

static cell_t native_GetParamString(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  try {
    auto str = context->ReadParam<std::string>(params[2]);
    pContext->StringToLocal(params[3], params[4], str.c_str());

    return 1;
  } catch (std::domain_error e) {
    return pContext->ThrowNativeError("Invalid convers, parameter %d is not a string", params[2]);
  }
}

static cell_t native_GetParamFloat(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  try {
    return sp_ftoc(context->ReadParam<float>(params[2]));
  } catch (std::domain_error e) {
    return pContext->ThrowNativeError("Invalid convers, parameter %d is not a float", params[2]);
  }
}

static cell_t native_GetParamBool(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  try {
    return context->ReadParam<bool>(params[2]);
  } catch (std::domain_error e) {
    return pContext->ThrowNativeError("Invalid convers, parameter %d is not a bool", params[2]);
  }
}

static cell_t native_FinishCall(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  context->finish();

  handlesys->FreeHandle(hndl, &sec);

  return 0;
}

static cell_t native_SetReturnInt(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  context->SetReturnValue<int>(params[2]);
  return 0;
}

static cell_t native_SetReturnFloat(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  context->SetReturnValue<float>(sp_ctof(params[2]));
  return 0;
}

static cell_t native_SetReturnBool(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  context->SetReturnValue<bool>(static_cast<bool>(params[2]));
  return 0;
}

static cell_t native_SetReturnString(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *str;
  pContext->LocalToString(params[2], &str);
  context->SetReturnValue<std::string>(std::string(str));
  return 0;
}

const sp_nativeinfo_t smrpc_context_natives[] = {
  {"RPCContext.GetParamInt", native_GetParamInt},
  {"RPCContext.GetParamFloat", native_GetParamFloat},
  {"RPCContext.GetParamString", native_GetParamString},
  {"RPCContext.GetParamBool", native_GetParamBool},
  {"RPCContext.SetReturnInt", native_SetReturnInt},
  {"RPCContext.SetReturnFloat", native_SetReturnFloat},
  {"RPCContext.SetReturnBool", native_SetReturnBool},
  {"RPCContext.SetReturnString", native_SetReturnString},
  {"RPCContext.Done", native_FinishCall},
  {NULL, NULL}
};