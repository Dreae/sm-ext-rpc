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

#ifndef _RPC_RPCContext
#define _RPC_RPCContext

#include <json.hpp>
#include "sdk/smsdk_ext.h"

using json = nlohmann::json;

extern HandleType_t g_RPCContextType;

class RPCContext {
  friend class RPCContextNatives;
private:
  json retval;
public:
  json params;
  std::string remote;

  RPCContext(const std::string &remote, json params, std::function<void(json)> callback);
  void finish();

  template <typename T> void SetReturnValue(T val) {
    retval = val;
  }

  std::function<void(json)> callback;
};

#endif // !_RPC_RPCContext
