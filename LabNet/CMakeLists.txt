cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML COMPONENTS audio REQUIRED)
find_path(BOOST_SIGNALS2_INCLUDE_DIRS "boost/signals2.hpp")
find_path(BOOST_ASIO_INCLUDE_DIRS "boost/asio.hpp")

add_executable(LabNet LabNet.cpp
               Log/easylogging++.cc Log/easylogging++.h Log/LoggingFacility.h Log/StandardOutputLogger.h Log/EasyLogger.h Log/LoggerFactory.h               
               network/Connection.cpp network/Connection.h network/ConnectionManager.cpp network/ConnectionManager.h network/Server.cpp network/Server.h
               network/LabNet.pb.h network/LabNetClient.pb.h network/LabNetServer.pb.h network/LabNet.pb.cc network/LabNetClient.pb.cc
               network/LabNetServer.pb.cc network/ProtocolAll.h network/shared_buffer.h
               network/server_messages.h network/server_actor.h network/server_actor.cpp
               Interface/uart/serial_port.cpp Interface/uart/serial_port.h  Interface/uart/serial_ports_manager.h
               Interface/uart/serial_ports_manager.cpp Interface/uart/private_messages.h Interface/uart/digital_input.h Interface/uart/digital_output.h 
               Interface/io_board/board_actor.h Interface/io_board/board_actor.cpp  Interface/io_board/digital_input_state_reader.cpp
               Interface/io_board/digital_input.h Interface/io_board/digital_output.h Interface/io_board/digital_input_state_reader.h
               Interface/io_board/resource_request_helper.h Interface/io_board/resource_request_helper.cpp
               Interface/rfid_board/data_read_worker.h Interface/rfid_board/max_14830.h
               Interface/rfid_board/max_14830_defs.h Interface/rfid_board/max_device.h Interface/rfid_board/spi.h Interface/rfid_board/data_read_worker.cpp
               Interface/rfid_board/max_14830.cpp Interface/rfid_board/max_device.cpp Interface/rfid_board/spi.cpp Interface/rfid_board/rfid_main_actor.cpp
               Interface/rfid_board/rfid_main_actor.h 
               Interface/gpio_wiring/gpio_manager.h Interface/gpio_wiring/digital_input.h Interface/gpio_wiring/digital_output.h Interface/gpio_wiring/digital_input_state_reader.h
               Interface/gpio_wiring/digital_input_state_reader.cpp Interface/gpio_wiring/gpio_manager.cpp
               Interface/gpio_wiring/resource_request_helper.h Interface/gpio_wiring/resource_request_helper.cpp
               Interface/sound/signal_generator.h Interface/sound/signal_generator.cpp Interface/sound/signals.h
               Interface/digital_messages.h Interface/stream_messages.h
               Interface/manage_interfaces.h Interface/interface_messages.h
               Interface/manage_interfaces.cpp Interface/interfaces.h
               DigitalOut/digital_out_helper.h DigitalOut/digital_out_helper.cpp DigitalOut/loop_messages.h DigitalOut/pulse_helper.h
               DigitalOut/pulse_helper.cpp DigitalOut/loop_helper.h DigitalOut/pin_id.h DigitalOut/loop_helper.cpp
               helper/reset_helper.h helper/reset_helper.cpp helper/reset_msg.h
               resources/resource.h resources/resource.cpp resources/resources_actor.h  resources/resources_actor.cpp)

target_include_directories(LabNet PRIVATE ${BOOST_ASIO_INCLUDE_DIRS} ${BOOST_SIGNALS2_INCLUDE_DIRS})
target_include_directories(LabNet PRIVATE ../sobjectizer/dev)
target_include_directories(LabNet PRIVATE Log)

target_link_libraries(LabNet atomic pthread boost_system wiringPi so.5.7.1 protobuf sfml-system sfml-audio)

add_compile_definitions(ELPP_THREAD_SAFE)
add_compile_definitions(ELPP_NO_DEFAULT_LOG_FILE)
FILE(COPY Log/logger.conf DESTINATION "${CMAKE_BINARY_DIR}")