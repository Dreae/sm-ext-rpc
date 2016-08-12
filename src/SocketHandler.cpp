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

      rpcCommandProcessor.HandleRequest(body, [this, self](std::shared_ptr<json> res) {
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
