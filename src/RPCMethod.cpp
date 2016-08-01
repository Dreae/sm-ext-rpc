#include "RPCMethod.hpp"

RPCMethod::RPCMethod(char *name, IPluginFunction* callback, ParamType returnType, std::unique_ptr<std::vector<ParamType>> paramTypes) {
  this->paramTypes = std::move(paramTypes);
  this->name = std::string(name);
  this->callback = callback;
  this->returnType = returnType;
}

void RPCMethod::Call(json params, std::function<void(json)> callback) {
  this->callback->PushCell(0);
  this->callback->PushCell(0);

  cell_t result;
  this->callback->Execute(&result);
  
  callback("42"_json);
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
  case ParamType::String:
    return j.is_string();
  }
}

RPCCallResult::RPCCallResult(bool success) {
  this->success = success;
}
