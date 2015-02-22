#ifndef NET_CONNECTION_H
#define NET_CONNECTION_H

#include "packet.h"
#include "buffer.h"

#include <memory>
#include <vector>
#include <mutex>

namespace net {

	const unsigned int MAX_CONNECTIONS = 8;

	class Connection;
	typedef std::shared_ptr<Connection> ConnectionPtr;

	class Buffer;

	class Connection {
	public:
		friend class Client;
		friend class Server;

		Connection(const std::shared_ptr<std::mutex>& mutex);

		bool receive(net::Packet& packet);

		void send(const net::Packet& packet);

		void stop();

		bool isActive() const;

		int getId() const;

	private:
		Buffer buffer_;
	};

} // Namespace net.

#endif // NET_CONNECTION_H
