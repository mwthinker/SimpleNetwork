#include "connection.h"
#include "network.h"

#include "buffer.h"

namespace net {

	Connection::Connection(const std::shared_ptr<std::mutex>& mutex) : buffer_(mutex) {
	}

	bool Connection::receive(net::Packet& packet) {
		return buffer_.popReceiveBuffer(packet);
	}

	void Connection::send(const net::Packet& packet) {
		if (packet.size() > 1) {
			buffer_.addToSendBuffer(packet);
		}
	}

	void Connection::stop() {
		buffer_.setActive(false);
	}

	bool Connection::isActive() const {
		return buffer_.isActive();
	}

	int Connection::getId() const {
		return buffer_.getId();
	}

} // Namespace net.
