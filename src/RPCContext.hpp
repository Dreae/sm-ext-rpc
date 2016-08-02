#ifndef _RPC_RPCContext
#define _RPC_RPCContext

#include <json.hpp>
#include "sdk/smsdk_ext.h"

using json = nlohmann::json;

extern HandleType_t g_RPCContextType;

class RPCContext {
private:
  json params;
public:
  RPCContext(json params, std::function<void(json)> callback);
  template <typename T> T ReadParam(int pos) {
    return this->params[pos].get<T>();
  };

  std::function<void(json)> callback;
};

#endif // !_RPC_RPCContext
