#ifndef _RPC_EventLoop
#define _RPC_EventLoop

#include <string>
#include <boost/asio.hpp>
#include "Exstension.hpp"

using boost::asio::ip::tcp;

class IOService {
  friend class EventLoop;
  virtual void Run() { };
};

class EventLoop : public SMRPCBase {
private:
  bool listening = false;
  boost::asio::io_service *ioService;
  std::string apiKey;
  tcp::acceptor *acceptor;
  tcp::socket *socket;
  void accept();
  std::vector<std::shared_ptr<IOService>> services;
public:
  EventLoop() { };
  void Init(std::string apiKey, int port);
  void RegisterService(std::shared_ptr<IOService> service);
  void Run();
  void OnExtLoad();
  void OnExtUnload();
};

extern EventLoop eventLoop;

#endif // !_RPC_EventLoop
