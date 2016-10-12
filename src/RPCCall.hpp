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

#ifndef _RPC_RPCCall
#define _RPC_RPCCall

#include <json.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "Exstension.hpp"

enum RPCReqResult {
  RPCReqResult_UnknownServer,
  RPCReqResult_Sent
};

using json = nlohmann::json;

class RPCCall {
  friend class RPCCallNatives;
private:
  std::string method;
  IPluginFunction *callback;
  std::shared_ptr<json> args;
  std::string id;
  Handle_t handle;
  IdentityToken_t *owner;
public:
  RPCCall(IPluginFunction *callback, IdentityToken_t *owner);
  void SetMethod(std::string method);
  std::string GetId();
  void SetCallback(IPluginFunction *callback);
  void SetHandle(Handle_t handle);
  void FreeHandle();
  void SetArgsJSON(json *j);
  void HandleReply(json *res);
  RPCReqResult Send(std::string server);
  RPCReqResult Notify(std::string server);
  void Broadcast();
};

extern boost::uuids::random_generator uuidGenerator;

#endif //_RPC_RPCCall
