#ifndef NET_NETWORK_H
#define NET_NETWORK_H

#include "packet.h"
#include "buffer.h"

#include <SDL_net.h>

#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include <thread>

namespace net {

    class Server;
    class Client;
    class Connection;
	class ConnectionsControl;

	enum Status {
		ACTIVE, NOT_ACTIVE
	};

	// This class works as a multi-user system. Should be used
	// to control a server/client system.
	class Network {
	public:
		Network();
		~Network();

		Network(const Network&) = delete;
		Network& operator=(const Network&) = delete;

		// Start the connection to the server.
		void startServer(int port);

		void startClient(std::string serverIp, int port);

		// End all active connections.
		void stop();

		void setAcceptConnections(bool accept);

		bool isAcceptingConnections() const;

		std::shared_ptr<Connection> pollConnection();

		// Return the current status for the network.
		Status getStatus() const;

	private:
		ConnectionsControl* connectionsControl_;

		std::shared_ptr<std::mutex> mutex_;
		std::thread thread_;

		Status status_;
		static bool firstInstance;
	};

} // Namespace net.

#endif // NET_NETWORK_H
