#include "CachedSocket.hpp"

CachedSocket::CachedSocket(std::unique_ptr<boost::asio::ip::tcp::socket> socket) {
  this->socket = std::move(socket);
}

bool CachedSocket::Connected() {

}