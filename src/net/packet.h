#ifndef NET_PACKET_H
#define NET_PACKET_H

#include <array>
#include <algorithm>

namespace net {

	// The class Packet represent a holder of formatted data. No dynamic data, is used.
	class Packet {
	public:
		static const int MAX_SIZE = 128;

		// Creates an empty packet with size ONE.
		// The one extra byte is used to store the size.
		// E.g:
		// Packet packet;
		// std::cout << "Example, size of empty packet: " << packet.getSize() << "." << std::endl;
		//
		// Run the example:
		// >> Example, size of empty packet: 1.
		inline Packet() : index_(1) {
			data_[0] = 1;
		}

		// Fill an empty packet with the data.
		// The size will be: getSize() == size + 1.
		// The one extra byte is used to store the size.
		inline Packet(const char* data, int size) : index_(1) {
			std::copy(data, data + size, data_.data());
		}

		inline Packet& operator>>(char& byte) {
			byte = data_[index_++];
			return *this;
		}

		inline Packet& operator<<(char byte) {
			pushBack(byte);
			return *this;
		}

		const char* getData() const {
			return data_.data();
		}

		inline int getSize() const {
			return data_[0];
		}

		inline void pushBack(char byte) {
			data_[data_[0]++] = byte;
		}

		inline char operator[](int index) const {
			return data_[index];
		}

		inline unsigned int dataLeftToRead() const {
			return data_[0] - index_;
		}
		
		inline void reset() {
			index_ = 1;
		}

		inline void clear() {
			index_ = 1;
			data_[0] = 1;
		}

	private:
		std::array<char, MAX_SIZE> data_;
		int index_; // The index for the next byte to be read.
	};

} // Namespace net.

#endif // NET_PACKET_H
