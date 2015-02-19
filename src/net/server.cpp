#include "server.h"
#include "connection.h"

#include <thread>

namespace net {

	Server::Server(const std::shared_ptr<std::mutex>& mutex) : socketSet_(0), buffer_(mutex),
        mutex_(mutex), active_(true) {

	}

	Server::~Server() {
		active_ = false;
		if (socketSet_ != nullptr) {
			SDLNet_FreeSocketSet(socketSet_);
		}
	}

	std::shared_ptr<Connection> Server::pollNewConnections() {
        if (!newConnections_.empty()) {
            mutex_->lock();
            auto connection = newConnections_.front();
            newConnections_.pop();
            mutex_->unlock();
            return connection;
        }
        return nullptr;
	}

	void Server::close() {
        active_ = true;
    }

	void Server::run(int port) {
		bool active = listen(port);

		if (active) {
			socketSet_ = SDLNet_AllocSocketSet(8);
		}

		while (active) {
			auto connection = handleNewConnection();

			// Receive data from sockets.
			receiveData();

			// Send data to sockets.
			sendData();

			mutex_->lock();
			if (connection) {
                newConnections_.push(connection);
			}
			active = active_;
			mutex_->unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	bool Server::listen(int port) {
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

	std::shared_ptr<Connection> Server::handleNewConnection() {
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

	void Server::receiveData() {
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

	void Server::sendData() {
		for (auto& pair : clients_) {
			std::array<char, Packet::MAX_SIZE> data;
			int size = buffer_.removeFromSendBufferTo(data);
			if (size > 0) {
				SDLNet_TCP_Send(pair.first, data.data(), size);
			}
		}
	}

} // Namespace net.
