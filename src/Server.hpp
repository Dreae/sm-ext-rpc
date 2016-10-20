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

#ifndef _RPC_Server
#define _RPC_Server

#include <boost/asio.hpp>
#include "EventLoop.hpp"

class Server : public IOService, public std::enable_shared_from_this<Server> {
private:
  std::unique_ptr<boost::asio::ip::tcp::socket> socket;
  std::unique_ptr<boost::asio::io_service> service;
  std::unique_ptr<boost::asio::io_service::work> work;
  int port;
  std::string address;
  bool connected = false;

  std::unique_ptr<boost::asio::streambuf> data = std::unique_ptr<boost::asio::streambuf>(new boost::asio::streambuf());
  void do_read();
public:
  Server(std::string address, int port);
  void Send(std::string msg);
  void Connect(std::function<void()> callback);
  void Run();
  int GetPort();
  const std::string &GetAddress();
};
#endif //_RPC_Server
