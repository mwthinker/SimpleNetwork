#ifndef NET_NETWORK_H
#define NET_NETWORK_H

#include "packet.h"
#include "buffer.h"
#include "connection.h"

#include <SDL_net.h>

#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <thread>
#include <mutex>

namespace net {

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
		void startServer(int port,
			const std::function<void(std::shared_ptr<Connection>)> connectionMade);

		void startClient(std::string serverIp, int port,
			const std::function<void(std::shared_ptr<Connection>)> connectionMade);

		// End all active connections.
		void stop();

		// Return the current status for the network.
		Status getStatus() const;

	private:
		class Client {
		public:
			Client(std::mutex& mutex);
			~Client();

			// Not thread safe. Closes the the thread as fast as it can.
			inline void close() {
				active_ = true;
			}

			void run(int port, std::string ip, const std::function<void(std::shared_ptr<Connection>)>& connectionMade);

		private:
			void receiveData();
			void sendData();

			IPaddress ip_;
			SDLNet_SocketSet socketSet_;
			Buffer buffer_;
			std::mutex& mutex_;
			TCPsocket socket_;
			bool active_;
		};

		class Server {
		public:
			Server(std::mutex& mutex);
			~Server();

			// Not thread safe. Closes the the thread as fast as it can.
			inline void close() {
				active_ = true;
			}
			
			void run(int port, const std::function<void(std::shared_ptr<Connection>)>& connectionMade);

		private:
			bool listen(int port);
			std::shared_ptr<Connection> handleNewConnection();
			void receiveData();
			void sendData();

			TCPsocket listenSocket_;
			SDLNet_SocketSet socketSet_;
			IPaddress ip_;
			
			std::map<TCPsocket, std::shared_ptr<Connection>> clients_;
			Buffer buffer_;
			std::mutex& mutex_;
			bool active_;
		};	

		Server* server_;
		Client* client_;
		std::mutex mutex_;
		std::function<void(std::shared_ptr<Connection>)> connectionMade_;
		Status status_;
		std::thread thread_;

		static bool firstInstance;
	};

} // Namespace net.

#endif // NET_NETWORK_H
