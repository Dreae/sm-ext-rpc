/**
 * Copyright 2019 Dreae
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

#ifndef _RPC_RPCMethod
#define _RPC_RPCMethod

#include <memory>
#include <vector>
#include "sdk/smsdk_ext.h"
#include "json.hpp"

using json = nlohmann::json;

enum class ParamType {
  String = 1,
  Int,
  Bool,
  Float,
  JSON
};

class RPCCallResult {
public:
  bool success;
  RPCCallResult(bool success);
};

class RPCMethod {
private:
  std::unique_ptr<std::vector<ParamType>> paramTypes;
  int paramCount;
  std::string name;
  IPluginFunction *callback;
  IPluginContext *owningPlugin;

  bool checkType(ParamType type, json j);
public:
  RPCMethod(char *name, IPluginContext *owningPlugin, IPluginFunction *callback, std::unique_ptr<std::vector<ParamType>> paramTypes);
  bool ValidateArguments(json j);
  void Call(const std::string &remote, json params, std::function<void(json)> callback);
};

#endif // !_RPC_RPCMethod
