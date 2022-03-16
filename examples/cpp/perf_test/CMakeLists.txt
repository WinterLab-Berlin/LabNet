cmake_minimum_required (VERSION 3.8)

project ("perf_test")

find_package(Boost COMPONENTS system)
find_package(sobjectizer REQUIRED)
find_package(protobuf REQUIRED)

add_executable(perf_test main.cpp client.h client.cpp shared_buffer.h stats.h stats.cpp
	prot/LabNet.pb.h prot/LabNetClient.pb.h prot/LabNetServer.pb.h prot/LabNet.pb.cc prot/LabNetClient.pb.cc
    prot/LabNetServer.pb.cc start_mes.h id_test.h set_dig_out_test.h set_and_read_dig_out_test.h "save_latencies.h")

target_include_directories(perf_test PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(perf_test protobuf::libprotobuf sobjectizer::SharedLib ${Boost_LIBRARIES})

if(WIN32)
    macro(get_WIN32_WINNT version)
        if(CMAKE_SYSTEM_VERSION)
            set(ver ${CMAKE_SYSTEM_VERSION})
            string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
            string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
            # Check for Windows 10, b/c we'll need to convert to hex 'A'.
            if("${verMajor}" MATCHES "10")
                set(verMajor "A")
                string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
            endif()
            # Remove all remaining '.' characters.
            string(REPLACE "." "" ver ${ver})
            # Prepend each digit with a zero.
            string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
            set(${version} "0x${ver}")
        endif()
    endmacro()

    get_WIN32_WINNT(ver)
    add_definitions(-D_WIN32_WINNT=${ver})
endif()