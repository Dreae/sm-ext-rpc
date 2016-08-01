#ifndef _RPC_SocketHandler
#define _RPC_SocketHandler
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class SocketHandler : public std::enable_shared_from_this<SocketHandler> {
private:
  tcp::socket socket;
  std::unique_ptr<boost::asio::streambuf> data = std::unique_ptr<boost::asio::streambuf>(new boost::asio::streambuf());
  void do_read();
  void do_write(std::string response);

public:
  SocketHandler(tcp::socket socket) : socket(std::move(socket)) { }
  void Start() {
    do_read();
  }
};

#endif // !_RPC_SocketHandler
