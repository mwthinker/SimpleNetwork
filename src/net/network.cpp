#include "packet.h"
#include "network.h"
#include "server.h"
#include "client.h"
#include "connection.h"

namespace net {

	bool Network::firstInstance = true;

	Network::Network() : status_(NOT_ACTIVE),
        client_(0), server_(0) {

		if (firstInstance) {
			firstInstance = false;
			SDLNet_Init();
		}
	}

	Network::~Network() {
		stop();
	}

	void Network::startServer(int port) {
	    if (client_ == 0 && server_ == 0) {
            mutex_ = std::make_shared<std::mutex>();
			server_ = new Server(mutex_);
			status_ = ACTIVE;
			thread_ = std::thread(&Server::run, server_, port);
		}
	}

	void Network::startClient(std::string serverIp, int port) {
		if (client_ == 0 && server_ == 0) {
            mutex_ = std::make_shared<std::mutex>();
			client_ = new Client(mutex_);
			status_ = ACTIVE;
			thread_ = std::thread(&Client::run, client_, port, serverIp);
		}
	}

	void Network::stop() {
	    // Has a active server/client?
	    if (client_ != 0 || server_ || 0) {
            if (client_ != 0) {
                client_->close();
            }
            if (server_ != 0) {
                server_->close();
            }
            status_ = DISCONNECTING;
            thread_.join();
            delete client_;
            delete server_;
            client_ = 0;
            server_ = 0;
            status_ = NOT_ACTIVE;
	    }
	}

	void Network::setAcceptConnections(bool accept) {
	    if (server_ != 0) {
            server_->setAcceptConnections(accept);
	    }
	}

	bool Network::isAcceptingConnections() const {
	    if (client_ != 0) {
            return true;
	    }
	    if (server_ != 0) {
            return server_->isAcceptingConnections();
	    }
	    return true;
	}

	std::shared_ptr<Connection> Network::pollConnection() {
	    if (client_ != 0) {
            return client_->pollConnection();
	    }
	    if (server_ != 0) {
            return server_->pollConnection();
	    }
        return nullptr;
	}

	Status Network::getStatus() const {
		return status_;
	}

} // Namespace net.
