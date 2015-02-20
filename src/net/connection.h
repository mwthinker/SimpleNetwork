#ifndef NET_CONNECTION_H
#define NET_CONNECTION_H

#include "packet.h"

#include <memory>
#include <vector>

namespace net {

	class Connection;
	typedef std::shared_ptr<Connection> ConnectionPtr;

	class Buffer;

	class Connection {
	public:
		Connection(Buffer& network);

		bool receive(net::Packet& packet);

		void send(const net::Packet& packet);

		void stop();

		bool isActive() const;

		int getId() const;

	private:
		Buffer& buffer_;
	};

} // Namespace net.

#endif // NET_CONNECTION_H
