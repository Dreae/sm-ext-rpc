#ifndef _RPC_CachedSocket
#define _RPC_CachedSocket

#include <boost/asio.hpp>

class CachedSocket {
  std::time_t expires;
public:
  std::unique_ptr<boost::asio::ip::tcp::socket> socket;
  CachedSocket(std::unique_ptr<boost::asio::ip::tcp::socket> socket);
  bool Connected();
  bool SetExpiry(std::time_t expires);
  bool CheckExpired();
};


#endif // !_RPC_CachedSocket