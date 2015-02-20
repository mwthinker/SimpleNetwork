#include <net/network.h>
#include <net/connection.h>

#include <string>
#include <sstream>
#include <cassert>
#include <iostream>
#include <list>

void test1() {
	const int SERVER_PORT = 12345;
	const std::string SERVER_IP("localhost");
	net::Network server;
	assert(net::NOT_ACTIVE == server.getStatus());
	server.startServer(SERVER_PORT);
	assert(net::ACTIVE == server.getStatus());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
	// Create clients.
    std::list<net::Network> clients;
	for (int i = 0; i < 10; ++i) {
		clients.emplace_back();
	}
	// Create clients.
	for (auto& client : clients) {
		client.startClient(SERVER_IP, SERVER_PORT);
	}

    std::list<net::ConnectionPtr> connections;
	while (connections.size() < 8) {
        auto connection = server.pollConnection();

        if (connection != 0) {
            std::cout <<"\nNew connection to the server!\n" << std::endl;
            connections.push_back(connection);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << ".";
        std::cout.flush();
	}


	std::cout << "test1, yes!" << std::endl;
}

void test2() {

}

void test3() {

}

void test4() {

}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_NOPARACHUTE);
	test1();
	test2();
	test3();
	test4();

	std::cout << "All test succeeded!\n";
	return 0;
}
