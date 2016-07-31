#ifndef _RPC_RPCMethod
#define _RPC_RPCMethod

#include <memory>
#include <vector>
#include "sdk\smsdk_ext.h"
#include "json.hpp"

using json = nlohmann::json;

enum class ParamType {
  String = 1,
  Int,
  Bool
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
  ParamType returnType;

  bool checkType(ParamType type, json j);
public:
  RPCMethod(char *name, IPluginFunction *callback, ParamType returnType, std::unique_ptr<std::vector<ParamType>> paramTypes);
  bool ValidateArguments(std::unique_ptr<json> j);
  std::unique_ptr<RPCCallResult> Call();
};
#endif // !_RPC_RPCMethod
