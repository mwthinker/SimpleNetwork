#ifndef NET_BUFFER_H
#define NET_BUFFER_H

#include "packet.h"

#include <mutex>
#include <vector>
#include <memory>

namespace net {

	class Buffer {
	public:
		Buffer(const std::shared_ptr<std::mutex>& mutex);

		bool popReceiveBuffer(Packet& packet);

		void addToSendBuffer(const Packet& packet);


		void addToReceiveBuffer(const std::array<char, 256>& data, int size);

		int removeFromSendBufferTo(std::array<char, Packet::MAX_SIZE>& data);

	private:
		std::vector<char> receiveBuffer_;
		std::vector<char> sendBuffer_;
		std::shared_ptr<std::mutex> mutex_;
		bool active_;
	};

} // Namespace net.

#endif // NET_BUFFER_H
