#include "packet.h"
#include "network.h"

#include <vector>

namespace net {

	bool Network::firstInstance = true;

	Network::Network() : status_(NOT_ACTIVE), client_(0), server_(0) {
		if (firstInstance) {
			firstInstance = false;
			SDL_Init(SDL_INIT_NOPARACHUTE);
			SDLNet_Init();
		}
	}

	Network::~Network() {
		stop();
	}

	void Network::startServer(int port,
		const std::function<void(std::shared_ptr<Connection>)> connectionMade) {

		if (client_ == 0 && server_ == 0) {
			connectionMade_ = connectionMade;
			server_ = new Server(mutex_);
			status_ = ACTIVE;
			thread_ = std::thread(&Network::Server::run, server_, port, connectionMade);
		}
	}

	void Network::startClient(std::string serverIp, int port,
		const std::function<void(std::shared_ptr<Connection>)> connectionMade) {

		if (client_ == 0 && server_ == 0) {
			connectionMade_ = connectionMade;
			client_ = new Client(mutex_);
			status_ = ACTIVE;
			thread_ = std::thread(&Network::Client::run, client_, port, serverIp, connectionMade);
		}
	}

	void Network::stop() {
		mutex_.lock();
		client_ ? client_->close() : client_ = 0;
		server_ ? server_->close() : server_ = 0;
		mutex_.unlock();
		status_ = DISCONNECTING;
		thread_.join();
		delete client_;
		delete server_;
		client_ = 0;
		server_ = 0;
		status_ = NOT_ACTIVE;
	}

	Status Network::getStatus() const {
		return status_;
	}

	Network::Client::Client(std::mutex& mutex) : socketSet_(0), buffer_(mutex), mutex_(mutex), active_(true) {
	}

	Network::Client::~Client() {
		active_ = false;
		if (socketSet_ != nullptr) {
			SDLNet_FreeSocketSet(socketSet_);
		}
	}

	void Network::Client::run(int port, std::string ip, const std::function<void(std::shared_ptr<Connection>)>& connectionMade) {
		if (SDLNet_ResolveHost(&ip_, ip.c_str(), port) < 0) {
			fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
			return;
		}

		socket_ = SDLNet_TCP_Open(&ip_);

		if (socket_) {
			socketSet_ = SDLNet_AllocSocketSet(1);
			SDLNet_TCP_AddSocket(socketSet_, socket_);
			bool active = true;

			mutex_.lock();
			connectionMade(std::make_shared<Connection>(buffer_));
			mutex_.unlock();
			
			while (active) {
				// Receive data from server.
				receiveData();

				// Send data to the server.
				sendData();

				mutex_.lock();
				active = active_;
				mutex_.unlock();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}
	}

	void Network::Client::receiveData() {
		while (SDLNet_CheckSockets(socketSet_, 0) > 0) {
			// Is ready to receive data?
			if (SDLNet_SocketReady(socket_) != 0) {
				std::array<char, 256> data;
				int size = SDLNet_TCP_Recv(socket_, data.data(), sizeof(data));
				if (size > 0) {
					buffer_.addToReceiveBuffer(data, size);
				}
			}
		}
	}

	void Network::Client::sendData() {
		std::array<char, Packet::MAX_SIZE> data;
		int size = buffer_.removeFromSendBufferTo(data);
		if (size > 0) {
			SDLNet_TCP_Send(socket_, data.data(), size);
		}
	}

	Network::Server::Server(std::mutex& mutex) : socketSet_(0), buffer_(mutex), mutex_(mutex), active_(true) {
	}

	Network::Server::~Server() {
		active_ = false;
		if (socketSet_ != nullptr) {
			SDLNet_FreeSocketSet(socketSet_);
		}
	}

	void Network::Server::run(int port, const std::function<void(std::shared_ptr<Connection>)>& connectionMade) {
		bool active = listen(port);

		if (active) {
			socketSet_ = SDLNet_AllocSocketSet(8);
		}

		while (active) {
			auto connection = handleNewConnection();
			if (connection) {
				mutex_.lock();
				connectionMade(connection);
				mutex_.unlock();
			}

			// Receive data from sockets.
			receiveData();

			// Send data to sockets.
			sendData();
			
			mutex_.lock();
			active = active_;
			mutex_.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	bool Network::Server::listen(int port) {
		// Resolving the host using NULL make network interface to listen.
		if (SDLNet_ResolveHost(&ip_, NULL, port) < 0) {
			fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
			return false;
		}

		// Open a connection with the IP provided (listen on the host's port).
		listenSocket_ = SDLNet_TCP_Open(&ip_);
		if (listenSocket_ == nullptr) {
			fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
			return false;
		}
		return true;
	}

	std::shared_ptr<Connection> Network::Server::handleNewConnection() {
		// New connection?
		if (TCPsocket socket = SDLNet_TCP_Accept(listenSocket_)) {
			if (IPaddress* remoteIP_ = SDLNet_TCP_GetPeerAddress(socket)) {
				SDLNet_TCP_AddSocket(socketSet_, socket);
				auto connection = std::make_shared<Connection>(buffer_);
				clients_[socket] = connection;
				return connection;
			} else {
				fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
			}
		}
		return nullptr;
	}

	void Network::Server::receiveData() {
		while (SDLNet_CheckSockets(socketSet_, 0) > 0) {
			for (auto& pair : clients_) {
				TCPsocket socket = pair.first;
				// Is ready to receive data?
				if (SDLNet_SocketReady(socket) != 0) {
					std::array<char, 256> data;
					int size = SDLNet_TCP_Recv(socket, data.data(), sizeof(data));
					if (size > 0) {
						buffer_.addToReceiveBuffer(data, size);
					}
				}
			}
		}
	}

	void Network::Server::sendData() {
		for (auto& pair : clients_) {
			std::array<char, Packet::MAX_SIZE> data;
			int size = buffer_.removeFromSendBufferTo(data);
			if (size > 0) {
				SDLNet_TCP_Send(pair.first, data.data(), size);
			}
		}
	}

} // Namespace net.
