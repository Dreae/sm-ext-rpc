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

#include "RPCContext.hpp"

RPCContext::RPCContext(const std::string &remote, json params, std::function<void(json)> callback) {
  this->params = params;
  this->callback = callback;
  this->remote = std::string(remote);
}

void RPCContext::finish() {
  this->callback(this->retval);
}