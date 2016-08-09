#include "Exstension.hpp"
#include "rpc_handletypes.hpp"
#include <json.hpp>

using json = nlohmann::json;

#define READ_HANDLE(pContext, params) \
  Handle_t hndl = static_cast<Handle_t>(params[1]); \
  HandleSecurity sec; \
  json *obj; \
  sec.pOwner = pContext->GetIdentity(); \
  sec.pIdentity = myself->GetIdentity(); \
  auto herr = handlesys->ReadHandle(hndl, g_JSONType, &sec, reinterpret_cast<void **>(&obj)); \
  if (herr != HandleError_None) { \
    return pContext->ThrowNativeError("Invalid JSON handle %x (error %d)", hndl, herr); \
  }


HandleType_t g_JSONType;
extern const sp_nativeinfo_t smrpc_json_natives[];

class RPCJSONNatives : public SMRPCBase, public IHandleTypeDispatch {
public:
  void OnExtLoad() {
    HandleAccess hacc;
    TypeAccess tacc;

    handlesys->InitAccessDefaults(&tacc, &hacc);
    tacc.ident = myself->GetIdentity();
    hacc.access[HandleAccess_Read] = HANDLE_RESTRICT_OWNER;
    tacc.access[HTypeAccess_Create] = true;
    tacc.access[HTypeAccess_Inherit] = true;

    g_JSONType = handlesys->CreateType("JSON", this, 0, &tacc, &hacc, myself->GetIdentity(), NULL);
    sharesys->AddNatives(myself, smrpc_json_natives);
  }

  void OnExtUnload() {
    handlesys->RemoveType(g_JSONType, myself->GetIdentity());
  }

  void OnHandleDestroy(HandleType_t type, void *object) {
    delete reinterpret_cast<json *>(object);
  }

  // Used by HandleSys for reporting during Dump() and when freeing handles
  // after detecting a memory leak
  bool GetHandleApproxSize(HandleType_t type, void *object, unsigned int *size) {
    json *obj = reinterpret_cast<json *>(object);
    *size = sizeof(json) + obj->size();

    return true;
  }
};

static RPCJSONNatives natives;

static cell_t native_SetJSONString(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *key;
  pContext->LocalToString(params[2], &key);

  char *value;
  pContext->LocalToString(params[3], &value);

  (*obj)[key] = std::string(value);
  return 1;
}

static cell_t native_SetJSONInt(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *key;
  pContext->LocalToString(params[2], &key);

  (*obj)[key] = params[3];
  return 1;
}

static cell_t native_SetJSONFloat(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *key;
  pContext->LocalToString(params[2], &key);

  (*obj)[key] = sp_ctof(params[3]);
  return 1;
}

static cell_t native_SetJSONBool(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *key;
  pContext->LocalToString(params[2], &key);

  (*obj)[key] = static_cast<bool>(params[3]);
  return 1;
}

static cell_t native_SetJSON_JSON(IPluginContext *pContext, const cell_t *params) {
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

  (*obj)[key] = *obj2;
  return 1;
}

static cell_t native_CreateJSON(IPluginContext *pContext, const cell_t *params) {
  auto context = new json;
  auto hndl = handlesys->CreateHandle(g_JSONType, context, pContext->GetIdentity(), myself->GetIdentity(), NULL);

  return hndl;
}

const sp_nativeinfo_t smrpc_json_natives[] = {
  { "JSON.JSON", native_CreateJSON },
  { "JSON.SetInt", native_SetJSONInt },
  { "JSON.SetFloat", native_SetJSONFloat },
  { "JSON.SetBool", native_SetJSONBool },
  { "JSON.SetString", native_SetJSONString },
  { "JSON.SetJSON", native_SetJSON_JSON },
  { NULL, NULL }
};