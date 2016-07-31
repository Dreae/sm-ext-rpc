#include "SocketHandler.hpp"
#include "sdk/smsdk_ext.h"

void SocketHandler::do_read() {
  auto self(shared_from_this());
  // TODO: Newline for testing, change to EOT when client code is done
  boost::asio::async_read_until(socket, *this->data, '\n', [this, self](boost::system::error_code ec, std::size_t length) {
    if (!ec) {
      this->do_write();
    } else {
      smutils->LogError(myself, "Boost error: %s", ec.message());
    }
  });
}

void SocketHandler::do_write() {
  auto self(shared_from_this());
  boost::asio::async_write(socket, boost::asio::buffer(data->data(), data->size()), [this, self](boost::system::error_code ec, std::size_t length) {
    if (!ec) {
      this->data = std::unique_ptr<boost::asio::streambuf>(new boost::asio::streambuf());
      this->do_read();
    } else {
      smutils->LogError(myself, "Boost error: %s", ec.message());
    }
  });
}
