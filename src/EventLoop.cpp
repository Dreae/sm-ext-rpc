#include "EventLoop.hpp"
#include "SocketHandler.hpp"

EventLoop eventLoop;

void GameFrame(bool simulating) {
  eventLoop.Run();
}

void EventLoop::OnExtLoad() {
  smutils->AddGameFrameHook(&GameFrame);
}

void EventLoop::OnExtUnload() {
  smutils->RemoveGameFrameHook(&GameFrame);
}

void EventLoop::Init(std::string apiKey, int port) {
  this->apiKey = apiKey;
  this->ioService = new boost::asio::io_service();
  this->socket = new boost::asio::ip::tcp::socket(*this->ioService);
  this->acceptor = new boost::asio::ip::tcp::acceptor(*this->ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
  this->accept();
}

void EventLoop::RegisterService(std::shared_ptr<IOService> service) {
  this->services.push_back(service);
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
  for (auto service : this->services) {
    service->Run();
  }
}
