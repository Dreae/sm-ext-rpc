#include "SocketHandler.hpp"
#include "sdk/smsdk_ext.h"

void SocketHandler::do_read() {
	auto self(shared_from_this());
	socket.async_read_some(boost::asio::buffer(data, 1024), [this, self](boost::system::error_code ec, std::size_t length) {
		if (!ec) {
			this->do_write(length);
		} else {
			smutils->LogError(myself, "Boost error: %s", ec.message());
		}
	});
}

void SocketHandler::do_write(size_t length) {
	auto self(shared_from_this());
	socket.async_write_some(boost::asio::buffer(data, length), [this, self](boost::system::error_code ec, std::size_t length) {
		if (!ec) {
			this->do_read();
		} else {
			smutils->LogError(myself, "Boost error: %s", ec.message());
		}
	});
}