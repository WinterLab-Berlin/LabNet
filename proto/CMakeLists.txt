cmake_minimum_required(VERSION 3.18.0)

include(FindProtobuf)
find_package(Protobuf REQUIRED) 
include_directories(${PROTOBUF_INCLUDE_DIR})


set(sources ${sources}
	LabNetClient.proto
	LabNet.proto
	LabNetServer.proto)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER ${sources})
add_library(proto ${PROTO_HEADER} ${PROTO_SRC})
target_include_directories(proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
