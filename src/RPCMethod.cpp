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

#include "Exstension.hpp"
#include "RPCMethod.hpp"
#include "RPCContext.hpp"

RPCMethod::RPCMethod(char *name, IPluginContext *owningPlugin, IPluginFunction* callback, std::unique_ptr<std::vector<ParamType>> paramTypes) {
  this->paramTypes = std::move(paramTypes);
  this->name = std::string(name);
  this->owningPlugin = owningPlugin;
  this->callback = callback;
}

void RPCMethod::Call(json params, std::function<void(json)> callback) {
  auto context = new RPCContext(params, callback); // Free'd by SourceMod in OnHandleDestroy
  auto hndl = handlesys->CreateHandle(g_RPCContextType, context, this->owningPlugin->GetIdentity(), myself->GetIdentity(), NULL);
  this->callback->PushCell(hndl);

  cell_t result;
  this->callback->Execute(&result);
}

bool RPCMethod::ValidateArguments(json j) {
  if (j.size() != this->paramTypes->size()) {
    return false;
  }

  for (unsigned int c = 0; c < this->paramTypes->size(); c++) {
    if (!this->checkType(this->paramTypes->at(c), j.at(c))) {
      return false;
    }
  }
}

bool RPCMethod::checkType(ParamType type, json j) {
  switch (type) {
  case ParamType::Bool:
    return j.is_boolean();
  case ParamType::Int:
    return j.is_number_integer();
  case ParamType::Float:
    return j.is_number();
  case ParamType::String:
    return j.is_string();
  case ParamType::JSON:
    return true;  
  }
}

RPCCallResult::RPCCallResult(bool success) {
  this->success = success;
}
