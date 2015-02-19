#include "packet.h"
#include "network.h"
#include "server.h"
#include "client.h"
#include "connection.h"

#include <array>

namespace net {

	bool Network::firstInstance = true;

	Network::Network() : status_(NOT_ACTIVE), client_(0),
        server_(0), mutex_(std::make_shared<std::mutex>()) {

		if (firstInstance) {
			firstInstance = false;
			SDL_Init(SDL_INIT_NOPARACHUTE);
			SDLNet_Init();
		}
	}

	Network::~Network() {
		stop();
	}

	void Network::startServer(int port) {
	    if (client_ == 0 && server_ == 0) {
			server_ = new Server(mutex_);
			status_ = ACTIVE;
			thread_ = std::thread(&Server::run, server_, port);
		}
	}

	void Network::startClient(std::string serverIp, int port) {
		if (client_ == 0 && server_ == 0) {
			client_ = new Client(mutex_);
			status_ = ACTIVE;
			thread_ = std::thread(&Client::run, client_, port, serverIp);
		}
	}

	void Network::stop() {
		mutex_->lock();
		if (client_ != nullptr) {
            client_->close();
		}
		if (server_ != nullptr) {
            server_->close();
		}
		mutex_->unlock();
		status_ = DISCONNECTING;
		thread_.join();
		delete client_;
		delete server_;
		client_ = 0;
		server_ = 0;
		status_ = NOT_ACTIVE;
		thread_ = std::thread();
	}

	std::shared_ptr<Connection> Network::pollNewConnections() {
	    if (client_ != 0) {
            return client_->pollNewConnections();
	    } else if (server_ != 0) {
            return server_->pollNewConnections();
	    }
        return nullptr;
	}

	Status Network::getStatus() const {
		return status_;
	}

} // Namespace net.
