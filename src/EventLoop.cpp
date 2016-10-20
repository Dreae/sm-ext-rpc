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

void EventLoop::Init(const std::string &address, int port) {
  this->ioService = new boost::asio::io_service();
  this->socket = new boost::asio::ip::tcp::socket(*this->ioService);
  this->acceptor = new boost::asio::ip::tcp::acceptor(*this->ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port));
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
