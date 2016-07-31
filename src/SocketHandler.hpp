#ifndef _RPC_SocketHandler
#define _RPC_SocketHandler
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class SocketHandler : public std::enable_shared_from_this<SocketHandler> {
private:
  tcp::socket socket;
  char data[1024];
  void do_read();
  void do_write(std::size_t length);

public:
  SocketHandler(tcp::socket socket) : socket(std::move(socket)) { }
  void Start() {
    do_read();
  }
};

#endif // !_RPC_SocketHandler
