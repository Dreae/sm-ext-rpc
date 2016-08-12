#ifndef _RPC_Server
#define _RPC_Server

#include <boost/asio.hpp>
#include "EventLoop.hpp"

class Server : public IOService, public std::enable_shared_from_this<Server> {
private:
  std::unique_ptr<boost::asio::ip::tcp::socket> socket;
  std::unique_ptr<boost::asio::io_service> service;
  std::unique_ptr<boost::asio::io_service::work> work;
  int port;
  std::string address;
  bool connected = false;
public:
  Server(std::string address, int port);
  void Send(std::string msg);
  void Connect(std::function<void()> callback);
  void Run();
};
#endif //_RPC_Server
