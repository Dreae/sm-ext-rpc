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
#include "RPCMethod.hpp"
#include "EventLoop.hpp"
#include "CommandProcessor.hpp"
#include "CoreConfig.hpp"
#include "Server.hpp"
#include <thread>

Extension extension;
SMEXT_LINK(&extension);

SMRPCBase *SMRPCBase::head = NULL;

bool Extension::SDK_OnLoad(char *error, size_t err_max, bool late) {
  config.Init();

  if (config.port <= 0 || config.port > 65535) {
    strcpy(error, "Invalid listen port provided in configuration");
    return false;
  } else if (config.address == "") {
    strcpy(error, "No listen address provided in configuration");
    return false;
  } else if (config.secret == "") {
    strcpy(error, "No signing key provided in configuration");
    return false;
  } else {
    eventLoop.Init(config.port);
    rpcCommandProcessor.Init(config.secret);

    for (auto n : config.servers) {
      auto server = std::make_shared<Server>(n.second->address, n.second->port);
      server->Connect(nullptr);
      eventLoop.RegisterService(server);
      rpcCommandProcessor.RegisterServer(n.first, server);
    }


    SMRPCBase *head = SMRPCBase::head;
    while (head) {
      head->OnExtLoad();
      head = head->next;
    }

    return true;
  }
}

void Extension::SDK_OnUnload() {
  SMRPCBase *head = SMRPCBase::head;
  while (head) {
    head->OnExtUnload();
    head = head->next;
  }
}