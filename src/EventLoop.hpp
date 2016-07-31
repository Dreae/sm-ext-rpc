#ifndef _RPC_EventLoop
#define _RPC_EventLoop

#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class EventLoop {
private:
  bool listening = false;
  boost::asio::io_service *ioService;
  std::string apiKey;
  tcp::acceptor *acceptor;
  tcp::socket *socket;
  void accept();
public:
  EventLoop() { };
  void Init(std::string apiKey, int port);
  void Run();
};

extern EventLoop eventLoop;

#endif // !_RPC_EventLoop
