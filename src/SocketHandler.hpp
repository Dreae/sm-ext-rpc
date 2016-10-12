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

#ifndef _RPC_SocketHandler
#define _RPC_SocketHandler
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class SocketHandler : public std::enable_shared_from_this<SocketHandler> {
private:
  tcp::socket socket;
  std::unique_ptr<boost::asio::streambuf> data = std::unique_ptr<boost::asio::streambuf>(new boost::asio::streambuf());
  void do_read();
  void do_write(std::string response);

public:
  SocketHandler(tcp::socket socket) : socket(std::move(socket)) { }
  void Start() {
    do_read();
  }
};

#endif // !_RPC_SocketHandler
