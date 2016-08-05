#ifndef _RPC_Server
#define _RPC_Server

#include <boost/asio.hpp>

class Server {
private:
  boost::asio::ip::tcp::socket socket;
public:
  Server(std::string address, int port);
};
#endif //_RPC_Server
