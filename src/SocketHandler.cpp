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

#include "SocketHandler.hpp"
#include "sdk/smsdk_ext.h"
#include <json.hpp>
#include "CommandProcessor.hpp"

using json = nlohmann::json;

void SocketHandler::do_read() {
  auto self(shared_from_this());
  boost::asio::async_read_until(socket, *this->data, PACKET_TERMINATOR, [this, self](boost::system::error_code ec, std::size_t length) {
    if (!ec) {
      std::string body(boost::asio::buffer_cast<const char*>(this->data->data()), length);
      auto remote = self->socket.remote_endpoint().address().to_string();

      rpcCommandProcessor.HandleRequest(remote, body, [this, self](std::shared_ptr<json> res) {
        this->do_write(res->dump());
      });

      this->data = std::unique_ptr<boost::asio::streambuf>(new boost::asio::streambuf());
      this->do_read();
    } else {
      smutils->LogError(myself, "Boost error: %s", ec.message());
    }
  });
}

void SocketHandler::do_write(std::string response) {
  auto self(shared_from_this());
  auto body = response + PACKET_TERMINATOR;
  boost::asio::async_write(socket, boost::asio::buffer(body, body.size()), [this, self](boost::system::error_code ec, std::size_t length) {
    if (ec) {
      smutils->LogError(myself, "Boost error: %s", ec.message());
    }
  });
}
