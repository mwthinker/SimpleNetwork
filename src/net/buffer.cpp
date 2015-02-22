#include "buffer.h"

namespace net {

	int Buffer::lastId_ = 0;

	Buffer::Buffer(const std::shared_ptr<std::mutex>& mutex) : mutex_(mutex),
		active_(true), id_(++lastId_) {

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
		sendBuffer_.insert(sendBuffer_.end(), packet.getData(), packet.getData() + packet.getSize());
		mutex_->unlock();
	}

	void Buffer::addToReceiveBuffer(char data) {
		mutex_->lock();
		receiveBuffer_.push_back(data);//insert(receiveBuffer_.end(), data.data(), data.data() + size);
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

	bool Buffer::isActive() const {
		mutex_->lock();
		bool active = active_;
		mutex_->unlock();
		return active;
	}

	void Buffer::setActive(bool active) {
		mutex_->lock();
		active_ = active;
		mutex_->unlock();
	}

} // Namespace net.
