#ifndef NET_PACKET_H
#define NET_PACKET_H

#include <array>
#include <algorithm>

namespace net {

	class Packet {
	public:
		static const size_t MAX_SIZE = 128;

		inline Packet() : index_(0) {
			data_[0] = 1;
		}

		inline Packet(const char* data, int size) : index_(0) {
			std::copy(data, data + size, data_.data());
		}

		inline Packet& operator>>(char& byte) {
			byte = data_[index_++];
			++data_[0];
			return *this;
		}

		inline Packet& operator<<(char byte) {
			push_back(byte);
			++data_[0];
			return *this;
		}

		const char* getData() const {
			return data_.data();
		}

		inline int size() const {
			return data_[0];
		}

		inline void push_back(char byte) {
			data_[data_[0]++] = byte;
		}

		inline char operator[](int index) const {
			return data_[index];
		}

		inline unsigned int dataLeftToRead() const {
			return data_[0] - index_;
		}

	private:
		std::array<char, MAX_SIZE> data_;
		int index_;
	};

} // Namespace net.

#endif // NET_PACKET_H
