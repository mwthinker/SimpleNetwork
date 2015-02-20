#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include "buffer.h"
#include "connection.h"
#include "connectionscontrol.h"

#include <SDL_net.h>

#include <mutex>
#include <functional>
#include <string>

namespace net {

	class Client : public ConnectionsControl {
    public:
        Client(const std::shared_ptr<std::mutex>& mutex);

        // Thread safe.
		std::shared_ptr<Connection> pollConnection() override;

		// Thread safe.
		inline void setAcceptConnections(bool accept) override {
		}

		// Thread safe.
		inline bool isAcceptingConnections() const override {
			return true;
		}

        // Thread safe. Closes the the thread as fast as it can.
		void close() override;

        // Should be run by a a other thread.
		void run(int port, std::string ip);

    private:
        void receiveData();
        void sendData();

        IPaddress ip_;
        SDLNet_SocketSet socketSet_;
        Buffer buffer_;
        TCPsocket socket_;
        std::shared_ptr<Connection> newConnection_;
        std::shared_ptr<Connection> connection_;
        std::shared_ptr<std::mutex> mutex_;
    };

} // Namespace net.

#endif // NET_CLIENT_H
