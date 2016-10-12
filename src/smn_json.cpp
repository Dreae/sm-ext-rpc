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

static cell_t native_GetJSONInt(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *key;
  pContext->LocalToString(params[2], &key);

  int res = (*obj)[key];
  return res;
}

static cell_t native_GetJSONFloat(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *key;
  pContext->LocalToString(params[2], &key);

  float res = (*obj)[key];
  return sp_ftoc(res);
}

static cell_t native_GetJSONBool(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *key;
  pContext->LocalToString(params[2], &key);

  bool res = (*obj)[key];
  return res;
}

static cell_t native_GetJSONString(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *key;
  pContext->LocalToString(params[2], &key);

  std::string res = (*obj)[key];
  pContext->StringToLocal(params[3], params[4], res.c_str());
  return 1;
}

static cell_t native_GetJSON_JSON(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *key;
  pContext->LocalToString(params[2], &key);

  json res = (*obj)[key];
  auto resPtr = new json(res);
  
  auto jsonHndle = handlesys->CreateHandle(g_JSONType, resPtr, pContext->GetIdentity(), myself->GetIdentity(), NULL);

  return jsonHndle;
}

static cell_t native_PushJSONString(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  char *string;
  pContext->LocalToString(params[2], &string);

  std::string strCopy(string);
  obj->push_back(strCopy);

  return 1;
}

static cell_t native_PushJSONInt(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  obj->push_back(params[1]);

  return 1;
}

static cell_t native_PushJSONFloat(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  obj->push_back(sp_ctof(params[1]));

  return 1;
}

static cell_t native_PushJSONBool(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  obj->push_back(static_cast<bool>(params[1]));

  return 1;
}

static cell_t native_PushJSON_JSON(IPluginContext *pContext, const cell_t *params) {
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

  obj->push_back(*obj2);
  return 1;
}

static cell_t native_GetArrayString(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  int pos = params[2];
  
  std::string res = (*obj)[pos];
  pContext->StringToLocal(params[3], params[4], res.c_str());
  return 1;
}

static cell_t native_GetArrayInt(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  int res = (*obj)[params[2]];
  return res;
}

static cell_t native_GetArrayFloat(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  float res = (*obj)[params[2]];
  return res;
}

static cell_t native_GetArrayBool(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  bool res = (*obj)[params[2]];
  return res;
}

static cell_t native_GetArrayJSON(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  json res = (*obj)[params[2]];
  auto jsonCpy = new json(res);

  auto jsonHndle = handlesys->CreateHandle(g_JSONType, jsonCpy, pContext->GetIdentity(), myself->GetIdentity(), NULL);
  return jsonHndle;
}

static cell_t native_GetArraySize(IPluginContext *pContext, const cell_t *params) {
  READ_HANDLE(pContext, params);

  return obj->size();
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
  { "JSON.GetInt", native_GetJSONInt },
  { "JSON.GetFloat", native_GetJSONFloat },
  { "JSON.GetBool", native_GetJSONBool },
  { "JSON.GetString", native_GetJSONString },
  { "JSON.GetJSON", native_GetJSON_JSON },
  { "JSON.PushString", native_PushJSONString },
  { "JSON.PushInt", native_PushJSONInt },
  { "JSON.PushFloat", native_PushJSONFloat },
  { "JSON.PushBool", native_PushJSONBool },
  { "JSON.PushJSON", native_PushJSON_JSON },
  { "JSON.GetArrayString", native_GetArrayString },
  { "JSON.GetArrayInt", native_GetArrayInt },
  { "JSON.GetArrayFloat", native_GetArrayFloat },
  { "JSON.GetArrayBool", native_GetArrayBool },
  { "JSON.GetArrayJSON", native_GetArrayJSON },
  { "JSON.GetArraySize", native_GetArraySize },
  { NULL, NULL }
};