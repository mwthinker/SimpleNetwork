#include <net/network.h>
#include <net/connection.h>

#include <string>
#include <sstream>
#include <cassert>
#include <iostream>
#include <list>

net::Packet& operator<<(net::Packet& packet, const std::string& text) {
	packet << (char) text.size();
	for (char c : text) {
		packet << c;
	}
	return packet;
}

net::Packet& operator>>(net::Packet& packet, std::string& text) {
	char size;
	packet >> size;
	for (int i = 0; i < size; ++i) {
		char c;
		packet >> c;
		text.push_back(c);
	}
	return packet;
}

const int SERVER_PORT = 12345;
const std::string SERVER_IP("localhost");

// Start the server and 10 clients.
void test1() {
	std::cout << "test1, start!" << std::endl;
	net::Network server(10);
	assert(net::NOT_ACTIVE == server.getStatus());
	server.startServer(SERVER_PORT);
	assert(net::ACTIVE == server.getStatus());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
	// Create clients.
    std::list<net::Network> clients;
	for (int i = 0; i < 10; ++i) {
		clients.emplace_back(10);
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
	int nbr = 0;
	for (auto& c : connections) {
		if (c->isActive()) {
			++nbr;
		}		
	}
	assert(connections.size() == nbr && nbr == net::MAX_CONNECTIONS);

	std::cout << "test1, yes!" << std::endl;
}

void test2() {
	std::cout << "test2, start!" << std::endl;
	net::Network server(10);
	server.startServer(SERVER_PORT);
	assert(net::ACTIVE == server.getStatus());
	net::Network client1(10);
	client1.startClient(SERVER_IP, SERVER_PORT);
	assert(net::ACTIVE == client1.getStatus());
	net::Network client2(10);
	assert(net::NOT_ACTIVE == client2.getStatus());
	client2.startClient(SERVER_IP, SERVER_PORT);
	
	std::list<net::ConnectionPtr> connections;
	while (connections.size() < 2) {
		auto connection = server.pollConnection();

		if (connection != 0) {
			std::cout << "\nNew connection to the server!\n" << std::endl;
			connections.push_back(connection);
		}
		std::cout << ".";
		std::cout.flush();
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	int nbr = 0;
	for (auto& c : connections) {
		if (c->isActive()) {
			++nbr;
		}
	}
	assert(connections.size() == nbr && nbr == 2);	

	auto c1 = client1.pollConnection();
	auto c2 = client2.pollConnection();

	assert(c1 != nullptr && c2 != nullptr);
	assert(c1->isActive() && c2->isActive());

	const std::string CLIENT_MESSAGE = "Hello, this is Client";
	{
		net::Packet packet;
		std::string message = CLIENT_MESSAGE;
		packet << message;
		c1->send(packet);
	}
	{
		net::Packet packet;
		std::string message = CLIENT_MESSAGE;
		packet << CLIENT_MESSAGE;
		c2->send(packet);
	}
	
	for (auto c : connections) {
		net::Packet packet;		
		while (!c->receive(packet)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
		std::string str;
		packet >> str;
		assert(str == CLIENT_MESSAGE);
	}

	std::cout << "test2, yes!" << std::endl;
}

void test3() {
	std::cout << "test3, start!" << std::endl;
	{
		net::Packet packet;
		assert(packet.getSize() == 1);
		packet << 'a' << 'b' << 'c';
		assert(packet.getSize() == 1 + 3);		
		assert(packet.dataLeftToRead() == 3);
		char c;
		packet >> c;
		assert(c == 'a');
		packet >> c;
		assert(c == 'b');
		packet >> c;
		assert(c == 'c');
		assert(packet.dataLeftToRead() == 0);

		assert(packet[0] == packet.getSize() && packet[0] == 4 && packet[1] == 'a' && packet[2] == 'b' && packet[3] == 'c');
	}
	
	std::cout << "test3, yes!" << std::endl;
}

void test4() {

}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_NOPARACHUTE);
	test3();
	//test1();
	test2();
	test4();

	std::cout << "All test succeeded!\n";
	return 0;
}
