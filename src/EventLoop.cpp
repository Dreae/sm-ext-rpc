#include "EventLoop.hpp"
#include "SocketHandler.hpp"
#include "sdk/smsdk_ext.h"

EventLoop eventLoop;

void EventLoop::Init(std::string apiKey, int port) {
  this->apiKey = apiKey;
  this->ioService = new boost::asio::io_service();
  this->socket = new boost::asio::ip::tcp::socket(*this->ioService);
  this->acceptor = new boost::asio::ip::tcp::acceptor(*this->ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
  this->accept();
}

void EventLoop::accept() {
  this->acceptor->async_accept(*this->socket, [this](boost::system::error_code ec) {
    if (!ec) {
      std::make_shared<SocketHandler>(std::move(*this->socket))->Start();
      this->accept();
    } else {
      smutils->LogError(myself, "Error accepting: %s", ec.message());
    }
  });
}

void EventLoop::Run() {
  this->ioService->poll();
}
