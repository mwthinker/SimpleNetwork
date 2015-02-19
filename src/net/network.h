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

	enum Status {
		ACTIVE, DISCONNECTING, NOT_ACTIVE
	};

	// This class works as a multi-user system. Should be used
	// to control a server/client system.
	class Network {
	public:
		Network();
		~Network();

		// Start the connection to the server.
		void startServer(int port);

		void startClient(std::string serverIp, int port);

		// End all active connections.
		void stop();

		std::shared_ptr<Connection> pollNewConnections();

		// Return the current status for the network.
		Status getStatus() const;

	private:
		Server* server_;
		Client* client_;

		std::shared_ptr<std::mutex> mutex_;
		std::thread thread_;

		Status status_;
		static bool firstInstance;
	};

} // Namespace net.

#endif // NET_NETWORK_H
