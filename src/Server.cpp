#include "Server.hpp"
#include "CommandProcessor.hpp"

Server::Server(std::string address, int port) {
  this->port = port;
  this->address = address;
  this->service = std::make_unique<boost::asio::io_service>();
  this->socket = std::make_unique<boost::asio::ip::tcp::socket>(*this->service);
  this->work = std::make_unique<boost::asio::io_service::work>(*this->service);
}

void Server::Connect(std::function<void()> callback) {
  // Resolver needs to stick around while the query is executed
  auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(*this->service);

  char sPort[8];
  snprintf(sPort, sizeof(sPort), "%hu", this->port);
  boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), address.c_str(), sPort);

  auto self(shared_from_this());
  resolver->async_resolve(query, [this, self, resolver, callback](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator i) {
    if (ec) {
      smutils->LogError(myself, "Error resolving %s: %s", this->address.c_str(), ec.message().c_str());
    } else {
      boost::asio::ip::tcp::endpoint ep(*i);
      this->socket->async_connect(ep, [this, self, callback](boost::system::error_code ec) {
        if (ec) {
          smutils->LogError(myself, "Error connecting to %s: %s", this->address.c_str(), ec.message().c_str());
        } else {
          LOG_MESSAGE("Connected to %s:%d", this->address.c_str(), this->port);
          this->connected = true;
          if(callback) {
            callback();
          }
        }
      });
    }
  });
}

void Server::Send(std::string msg) {
  auto self(shared_from_this());
  if(!this->connected) {
    this->Connect([msg, self, this]() {
        this->Send(msg);
    });
  } else {
    auto body = msg + PACKET_TERMINATOR;
    boost::asio::async_write(*this->socket, boost::asio::buffer(body, body.size()), [this, self](boost::system::error_code ec, std::size_t length) {
        if (ec) {
          smutils->LogError(myself, "Boost error: %s", ec.message());
        }
    });
  }
}

void Server::do_read() {
  auto self(shared_from_this());
  boost::asio::async_read_until(*this->socket, *this->data, PACKET_TERMINATOR, [this, self](boost::system::error_code ec, std::size_t length) {
      if (!ec) {
        std::string body(boost::asio::buffer_cast<const char*>(this->data->data()), length);
        rpcCommandProcessor.HandleReply(body);
        
        this->data = std::unique_ptr<boost::asio::streambuf>(new boost::asio::streambuf());
        this->do_read();
      } else {
        smutils->LogError(myself, "Boost error: %s", ec.message());
      }
  });
}

void Server::Run() {
  this->service->poll();
}