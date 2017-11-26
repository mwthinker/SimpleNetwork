#include "server.h"
#include "connection.h"

#include <thread>

namespace net {

	Server::Server(int sleepMilliseconds, const std::shared_ptr<std::mutex>& mutex) : socketSet_(0),
		mutex_(mutex), sleepTime_(sleepMilliseconds), active_(true), acceptConnection_(true) {

	}

	void Server::setAcceptConnections(bool accept) {
	    mutex_->lock();
	    acceptConnection_ = accept;
	    mutex_->unlock();
	}

	bool Server::isAcceptingConnections() const {
	    mutex_->lock();
	    bool accept = acceptConnection_;
	    mutex_->unlock();
	    return accept;
	}

	std::shared_ptr<Connection> Server::pollConnection() {
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
        mutex_->lock();
        active_ = false;
        mutex_->unlock();
    }

	void Server::run(int port) {
		bool active = listen(port);

		if (active) {
			socketSet_ = SDLNet_AllocSocketSet(MAX_CONNECTIONS);
		}

        bool acceptConnection = true;
		while (active) {
			// Remove the latest closed clients.
			for (auto it = clients_.begin(); it != clients_.end(); ++it) {
				if (!it->second->isActive()) {
					SDLNet_TCP_DelSocket(socketSet_, it->first);
					SDLNet_TCP_Close(it->first);
					it = clients_.erase(it);
				}
			}

            std::shared_ptr<Connection> connection;
            if (acceptConnection) {
                connection = handleNewConnection();
            }

			// Receive data from sockets.
			receiveData();

			// Send data to sockets.
			sendData();
			
			mutex_->lock();
			if (connection) {
                newConnections_.push(connection);
			}
			active = active_;
			acceptConnection = acceptConnection_;
			mutex_->unlock();

			std::this_thread::sleep_for(sleepTime_);
		}

		for (auto& pair : clients_) {
            SDLNet_TCP_DelSocket(socketSet_, pair.first);
            SDLNet_TCP_Close(pair.first);
		}
		clients_.clear();

		if (socketSet_ != nullptr) {
			SDLNet_FreeSocketSet(socketSet_);
		}
		socketSet_ = 0;

		if (listenSocket_ != 0) {
            SDLNet_TCP_Close(listenSocket_);
		}
		listenSocket_ = 0;
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
	    // The list of connections is not full?
		if (clients_.size() < MAX_CONNECTIONS) {
            // New connection?
            if (TCPsocket socket = SDLNet_TCP_Accept(listenSocket_)) {
                if (IPaddress* remoteIP_ = SDLNet_TCP_GetPeerAddress(socket)) {
                    SDLNet_TCP_AddSocket(socketSet_, socket);
                    auto connection = std::make_shared<Connection>(mutex_);
                    clients_[socket] = connection;
                    return connection;
                } else {
                    fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
                }
            }
		}
		return nullptr;
	}

	void Server::receiveData() {
		while (SDLNet_CheckSockets(socketSet_, 0) > 0) {
			for (auto& pair : clients_) {
				TCPsocket socket = pair.first;
				// Is ready to receive data?
				if (SDLNet_SocketReady(socket) != 0 && pair.second->isActive()) {
					//std::array<char, 2> data;
					char data;
					int size = SDLNet_TCP_Recv(socket, &data, 1);
					if (size > 0) {
						pair.second->buffer_.addToReceiveBuffer(data);
					} else { // Assume that the client disconnected.
						SDLNet_TCP_DelSocket(socketSet_, socket);
						SDLNet_TCP_Close(socket); // Removed from set, then closed!
						pair.second->stop();
						clients_.erase(socket); // Important to be called after ->stop(), 
						// shared pointer may otherwise be invalid.
					    break; // Iterator invalid!
					}
					break; // Socket found, abort searching after more activity.
				}
			}
		}
	}

	void Server::sendData() {
		for (auto& pair : clients_) {
			if (pair.second->isActive()) {
				net::Packet packet;
				if (pair.second->buffer_.popSendBuffer(packet)) {
					SDLNet_TCP_Send(pair.first, packet.getData(), packet.getSize());
				}
			}
		}
	}

} // Namespace net.
