#include "net/network.h"

#include <string>
#include <sstream>
#include <cassert>
#include <iostream>
#include <list>

void test1() {
	const int SERVER_PORT = 12346;
	const std::string SERVER_IP("localhost");
	net::Network server;
	assert(net::NOT_ACTIVE == server.getStatus());
	std::list<net::ConnectionPtr> connections;
	server.startServer(12346, [&](net::ConnectionPtr connection) {
		std::cout <<"New connection to the server!\n" << std::endl;
		connections.push_back(connection);
	});
	std::list<net::Network> clients;
	
	// Create clients.
	for (int i = 0; i < 10; ++i) {
		clients.emplace_back();
	}
	// Create clients.
	for (auto& client : clients) {
		client.startClient(SERVER_IP, SERVER_PORT, [&](net::ConnectionPtr connection) {});
	}

	while (connections.size() < 8) {
	}

	assert(net::ACTIVE == server.getStatus());
	std::cout << "test1, yes!" << std::endl;
}

void test2() {

}

void test3() {

}

void test4() {

}

int main(int argc, char** argv) {
	test1();
	test2();
	test3();
	test4();

	std::cout << "All test succeeded!\n";
	return 0;
}
