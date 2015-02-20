cmake_minimum_required(VERSION 2.8)
project(SimpleNetwork)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")
if (MSVC)
	# Exception handler model.
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -DWIN32)
else (MSVC)
	set(CMAKE_CXX_FLAGS "-std=c++11")
endif (MSVC)

option(SimpleNetworkTest "SimpleNetworkTest project is added" OFF)

# Source files.
set(SOURCES_NETWORK
	src/net/buffer.cpp
	src/net/buffer.h
	src/net/client.cpp
	src/net/client.h
	src/net/connection.cpp
	src/net/connection.h
	src/net/network.cpp
	src/net/network.h
	src/net/packet.h
	src/net/server.cpp
	src/net/server.h
	src/net/connectionscontrol.h
)

set(SOURCES_NETWORK_TEST
	srcTest/main.cpp
)
# End of source files.

find_package(SDL2 REQUIRED)
find_package(SDL2_net REQUIRED)
if (CMAKE_COMPILER_IS_GNUCXX)
	find_package(Threads REQUIRED) # Due to a bug, which forces the user to link to Threads when using #include <thread>.
	set(CMAKE_CXX_FLAGS_DEBUG)
endif (CMAKE_COMPILER_IS_GNUCXX)

include_directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2_NET_INCLUDE_DIRS}
)

add_library(SimpleNetwork ${SOURCES_NETWORK})

if (SimpleNetworkTest)
	include_directories(src)
	add_executable(SimpleNetworkTest ${SOURCES_NETWORK_TEST})

	if (MSVC)
		set_target_properties(SimpleNetworkTest PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt")
	endif (MSVC)

	target_link_libraries(SimpleNetworkTest
		SimpleNetwork
		${SDL2_LIBRARIES}
		${SDL2_NET_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
	)
endif (SimpleNetworkTest)
