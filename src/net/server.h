#ifndef NET_SERVER_H
#define NET_SERVER_H

#include "buffer.h"

#include <SDL_net.h>

#include <mutex>
#include <memory>
#include <map>
#include <functional>
#include <string>
#include <queue>

namespace net {

    class Connection;

    class Server {
    public:
        Server(const std::shared_ptr<std::mutex>& mutex);
        ~Server();

        std::shared_ptr<Connection> pollNewConnections();

        // Not thread safe. Closes the the thread as fast as it can.
        void close();

        void run(int port);

    private:
        bool listen(int port);
        std::shared_ptr<Connection> handleNewConnection();
        void receiveData();
        void sendData();

        TCPsocket listenSocket_;
        SDLNet_SocketSet socketSet_;
        IPaddress ip_;

        std::queue<std::shared_ptr<Connection>> newConnections_;
        std::map<TCPsocket, std::shared_ptr<Connection>> clients_;
        Buffer buffer_;
        std::shared_ptr<std::mutex> mutex_;
        bool active_;
    };

} // Namespace net.

#endif // NET_SERVER_H