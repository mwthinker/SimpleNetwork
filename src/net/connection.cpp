#include "connection.h"
#include "network.h"

#include "buffer.h"

namespace net {

	Connection::Connection(Buffer& buffer) : buffer_(buffer) {
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

	}

} // Namespace net.
