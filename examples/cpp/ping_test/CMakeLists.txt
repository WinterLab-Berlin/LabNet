cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project ("perf_test")

add_subdirectory(sobjectizer/dev)
find_package(Boost COMPONENTS system)

add_executable(perf_test main.cpp client.h client.cpp shared_buffer.h stats.h stats.cpp
	prot/LabNet.pb.h prot/LabNetClient.pb.h prot/LabNetServer.pb.h prot/LabNet.pb.cc prot/LabNetClient.pb.cc
    prot/LabNetServer.pb.cc start_mes.h id_test.h set_dig_out_test.h  "save_latencies.h")

target_include_directories(perf_test PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(perf_test protobuf so.5.7.4.1 ${Boost_LIBRARIES})
