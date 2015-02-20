#include "buffer.h"

namespace net {

	Buffer::Buffer(const std::shared_ptr<std::mutex>& mutex) : mutex_(mutex) {
	}

	bool Buffer::popReceiveBuffer(Packet& packet) {
		mutex_->lock();
		if (!receiveBuffer_.empty()) {
			unsigned int size = receiveBuffer_[0];
			if (receiveBuffer_.size() >= size) {
				packet = net::Packet(receiveBuffer_.data(), size);
				receiveBuffer_.erase(sendBuffer_.begin(), sendBuffer_.begin() + size);
				mutex_->unlock();
				return true;
			}
		}
		mutex_->unlock();
		return false;
	}

	void Buffer::addToSendBuffer(const Packet& packet) {
		mutex_->lock();
		sendBuffer_.insert(sendBuffer_.end(), packet.getData(), packet.getData() + packet.size());
		mutex_->unlock();
	}

	void Buffer::addToReceiveBuffer(const std::array<char, 256>& data, int size) {
		mutex_->lock();
		receiveBuffer_.insert(receiveBuffer_.end(), data.data(), data.data() + size);
		mutex_->unlock();
	}

	int Buffer::removeFromSendBufferTo(std::array<char, Packet::MAX_SIZE>& data) {
		mutex_->lock();
		if (sendBuffer_.empty()) {
			unsigned int size = sendBuffer_.size();
			if (size > data.size()) {
				size = data.size();
			}
			std::copy(sendBuffer_.begin(), sendBuffer_.begin() + size, data.data());
			sendBuffer_.erase(sendBuffer_.begin(), sendBuffer_.end());
			mutex_->unlock();
			return size;
		}
		mutex_->unlock();
		return 0;
	}

} // Namespace net.
