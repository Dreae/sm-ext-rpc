#include "Server.hpp"

Server::Server(std::string address, int port) {
  this->port = port;
  this->address = address;
  this->service = std::unique_ptr<boost::asio::io_service>(new boost::asio::io_service());
  this->socket = std::unique_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(*this->service));
}

void Server::Connect() {
  // Resolver needs to stick around while the query is executed
  auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(*this->service);

  char sPort[8];
  snprintf(sPort, sizeof(sPort), "%hu", this->port);
  boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), address.c_str(), sPort);

  auto self(shared_from_this());
  resolver->async_resolve(query, [this, self, resolver](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator i) {
    if (ec) {
      smutils->LogError(myself, "Error resolving %s: %s", this->address.c_str(), ec.message().c_str());
    } else {
      boost::asio::ip::tcp::endpoint ep(*i);
      this->socket->async_connect(ep, [this, self](boost::system::error_code ec) {
        if (ec) {
          smutils->LogError(myself, "Error connecting to %s: %s", this->address.c_str(), ec.message().c_str());
        } else {
          LOG_MESSAGE("Connected to %s:%d", this->address.c_str(), this->port);
        }
      });
    }
  });
}

void Server::Send(std::string msg) {
  auto self(shared_from_this());
  auto body = msg + PACKET_TERMINATOR;
  boost::asio::async_write(*this->socket, boost::asio::buffer(body, body.size()), [this, self](boost::system::error_code ec, std::size_t length) {
    if (ec) {
      smutils->LogError(myself, "Boost error: %s", ec.message());
    }
  });
}

void Server::Run() {
  this->service->poll();
}