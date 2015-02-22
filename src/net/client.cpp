#include "client.h"
#include "connection.h"
#include "buffer.h"

#include <thread>
#include <iostream>

namespace net {

	Client::Client(int sleepMilliseconds, const std::shared_ptr<std::mutex>& mutex) : socketSet_(0),
		connection_(std::make_shared<Connection>(mutex)), mutex_(mutex), sleepMilliseconds_(sleepMilliseconds) {

	}

	std::shared_ptr<Connection> Client::pollConnection() {
        if (newConnection_) {
            auto tmp = newConnection_;
            newConnection_ = nullptr;
            return tmp;
        }
        return nullptr;
	}

    void Client::close() {
		connection_->buffer_.setActive(false);
    }

	void Client::run(int port, std::string ip) {
		if (SDLNet_ResolveHost(&ip_, ip.c_str(), port) < 0) {
			fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
			return;
		}

		socket_ = SDLNet_TCP_Open(&ip_);

		if(socket_ == 0) {
            printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
            return;
        } else {
			socketSet_ = SDLNet_AllocSocketSet(1);
			SDLNet_TCP_AddSocket(socketSet_, socket_);
			bool active = true;

			mutex_->lock();
			newConnection_ = connection_;
			mutex_->unlock();

			while (active) {
				// Receive data from server.
				receiveData();

				// Send data to the server.
				sendData();
								
				active = connection_->buffer_.isActive();
				if (!active && sleepMilliseconds_ >= 0) {
					std::this_thread::sleep_for(std::chrono::milliseconds(sleepMilliseconds_));
				}
			}
		}
		if (socket_ != 0) {
            SDLNet_TCP_Close(socket_);
		}
		socket_ = 0;
		if (socketSet_ != 0) {
			SDLNet_FreeSocketSet(socketSet_);
		}
		socketSet_ = 0;
	}

	void Client::receiveData() {
		while (SDLNet_CheckSockets(socketSet_, 0) > 0) {
			// Is ready to receive data?
			if (SDLNet_SocketReady(socket_) != 0 && connection_->isActive()) {
				//std::array<char, 256> data;
				char data;
				int size = SDLNet_TCP_Recv(socket_, &data, 1);
				if (size > 0) {
					connection_->buffer_.addToReceiveBuffer(data);
				} else { // Assume that the client was disconnected.
					SDLNet_TCP_DelSocket(socketSet_, socket_);
					SDLNet_TCP_Close(socket_); // Removed from set, then closed!
					socket_ = 0;
					close();
					break; // Due, iterator invalid!
				}
			}
		}
	}

	void Client::sendData() {
		if (socket_ != 0 && connection_->isActive()) {
			net::Packet packet;
			if (connection_->buffer_.popSendBuffer(packet)) {
				SDLNet_TCP_Send(socket_, packet.getData(), packet.getSize());
            }
	    }
	}

} // Namespace net.
