cmake_minimum_required(VERSION 2.8)
project(SimpleNetwork)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")
if (MSVC)
	# Exception handler model.
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -DWIN32)
else (MSVC)
	set(CMAKE_CXX_FLAGS "-std=c++11")
endif (MSVC)

option(SimpleNetworksTest "SimpleNetworkTest project is added" OFF)

# Source files.
set(SOURCES_NETWORK
	src/net/buffer.cpp
	src/net/buffer.h
	src/net/connection.cpp
	src/net/connection.h
	src/net/network.cpp
	src/net/network.h
	src/net/packet.h
)

set(SOURCES_NETWORK_TEST
	srcTest/main.cpp
)
# End of source files.

find_package(SDL2 REQUIRED)
find_package(SDL2_net REQUIRED)

include_directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2_NET_INCLUDE_DIRS}
)

add_library(SimpleNetwork ${SOURCES_NETWORK})

if (MwLibrary2Test)
	include_directories(src)
	add_executable(SimpleNetworkTest ${SOURCES_NETWORK_TEST})

	if (MSVC)
		set_target_properties(SimpleNetworkTest PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt")
	endif (MSVC)

	target_link_libraries(SimpleNetworkTest
		Network
		${SDL2_LIBRARIES}
		${SDL2_NET_LIBRARIES}
	)
endif (MwLibrary2Test)
