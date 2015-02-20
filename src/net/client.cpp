#include "client.h"
#include "connection.h"

#include <thread>
#include <iostream>

namespace net {

	Client::Client(const std::shared_ptr<std::mutex>& mutex) : socketSet_(0), buffer_(mutex),
        mutex_(mutex) {

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
		buffer_.setActive(false);
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
			connection_ = std::make_shared<Connection>(buffer_);
			newConnection_ = connection_;
			mutex_->unlock();

			while (active) {
				// Receive data from server.
				receiveData();

				// Send data to the server.
				sendData();
								
				active = buffer_.isActive();
				if (!active) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
			if (SDLNet_SocketReady(socket_) != 0) {
				std::array<char, 256> data;
				int size = SDLNet_TCP_Recv(socket_, data.data(), sizeof(data));
				if (size > 0) {
					buffer_.addToReceiveBuffer(data, size);
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
	    if (socket_ != 0) {
            std::array<char, Packet::MAX_SIZE> data;
            int size = buffer_.removeFromSendBufferTo(data);
            if (size > 0) {
                SDLNet_TCP_Send(socket_, data.data(), size);
            }
	    }
	}

} // Namespace net.
