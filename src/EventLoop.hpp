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

#ifndef _RPC_EventLoop
#define _RPC_EventLoop

#include <string>
#include <boost/asio.hpp>
#include "Exstension.hpp"

using boost::asio::ip::tcp;

class IOService {
  friend class EventLoop;
  virtual void Run() { };
};

class EventLoop : public SMRPCBase {
private:
  bool listening = false;
  boost::asio::io_service *ioService;
  tcp::acceptor *acceptor;
  tcp::socket *socket;
  void accept();
  std::vector<std::shared_ptr<IOService>> services;
public:
  EventLoop() { };
  void Init(int port);
  void RegisterService(std::shared_ptr<IOService> service);
  void Run();
  void OnExtLoad();
  void OnExtUnload();
};

extern EventLoop eventLoop;

#endif // !_RPC_EventLoop
