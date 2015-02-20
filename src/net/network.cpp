#include "packet.h"
#include "network.h"
#include "server.h"
#include "client.h"
#include "connection.h"
#include "connectionscontrol.h"

namespace net {

	bool Network::firstInstance = true;

	Network::Network() : status_(NOT_ACTIVE),
		connectionsControl_(0) {

		if (firstInstance) {
			firstInstance = false;
			SDLNet_Init();
		}
	}

	Network::~Network() {
		stop();
	}

	void Network::startServer(int port) {
		if (connectionsControl_ == 0) {
            mutex_ = std::make_shared<std::mutex>();
			auto server = new Server(mutex_);
			status_ = ACTIVE;
			thread_ = std::thread(&Server::run, server, port);
			connectionsControl_ = server;
		}
	}

	void Network::startClient(std::string serverIp, int port) {
		if (connectionsControl_ == 0) {
            mutex_ = std::make_shared<std::mutex>();
			auto client = new Client(mutex_);
			status_ = ACTIVE;
			thread_ = std::thread(&Client::run, client, port, serverIp);
			connectionsControl_ = client;
		}
	}

	void Network::stop() {
	    // Has a active server/client?
		if (connectionsControl_ != 0) {
			connectionsControl_->close();
            thread_.join();
			delete connectionsControl_;
			connectionsControl_ = 0;
            status_ = NOT_ACTIVE;
	    }
	}

	void Network::setAcceptConnections(bool accept) {
		if (connectionsControl_ != 0) {
			connectionsControl_->setAcceptConnections(accept);
	    }
	}

	bool Network::isAcceptingConnections() const {
		if (connectionsControl_ != 0) {
			return connectionsControl_->isAcceptingConnections();
	    }
	    return true;
	}

	std::shared_ptr<Connection> Network::pollConnection() {
		if (connectionsControl_ != 0) {
			return connectionsControl_->pollConnection();
	    }
        return nullptr;
	}

	Status Network::getStatus() const {
		return status_;
	}

} // Namespace net.
