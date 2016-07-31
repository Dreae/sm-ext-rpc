#include "RPCMethod.hpp"

RPCMethod::RPCMethod(char *name, IPluginFunction* callback, ParamType returnType, std::unique_ptr<std::vector<ParamType>> paramTypes) {
  this->paramTypes = std::move(paramTypes);
  this->name = std::string(name);
  this->callback = callback;
  this->returnType = returnType;
}

std::unique_ptr<RPCCallResult> RPCMethod::Call() {
  return std::unique_ptr<RPCCallResult>(new RPCCallResult(false));
}

bool RPCMethod::ValidateArguments(std::unique_ptr<json> j) {
  if (j->size() != this->paramTypes->size()) {
    return false;
  }

  for (unsigned int c = 0; c < this->paramTypes->size(); c++) {
    if (!this->checkType(this->paramTypes->at(c), j->at(c))) {
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
