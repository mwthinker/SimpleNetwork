#include "buffer.h"

namespace net {

	Buffer::Buffer(std::mutex& mutex) : mutex_(mutex) {
	}

	bool Buffer::popReceiveBuffer(Packet& packet) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (!receiveBuffer_.empty()) {
			unsigned int size = receiveBuffer_[0];
			if (receiveBuffer_.size() >= size) {
				packet = net::Packet(receiveBuffer_.data(), size);
				receiveBuffer_.erase(sendBuffer_.begin(), sendBuffer_.begin() + size);
				return true;
			}
		}
		return false;
	}

	void Buffer::addToSendBuffer(const Packet& packet) {
		std::lock_guard<std::mutex> lock(mutex_);
		sendBuffer_.insert(sendBuffer_.end(), packet.getData(), packet.getData() + packet.size());
	}

	void Buffer::addToReceiveBuffer(const std::array<char, 256>& data, int size) {
		std::lock_guard<std::mutex> lock(mutex_);
		receiveBuffer_.insert(receiveBuffer_.end(), data.data(), data.data() + size);
	}

	int Buffer::removeFromSendBufferTo(std::array<char, Packet::MAX_SIZE>& data) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (sendBuffer_.empty()) {
			unsigned int size = sendBuffer_.size();
			if (size > data.size()) {
				size = data.size();
			}
			std::copy(sendBuffer_.begin(), sendBuffer_.begin() + size, data.data());
			sendBuffer_.erase(sendBuffer_.begin(), sendBuffer_.end());
			return size;
		}
		return 0;
	}

} // Namespace net.
