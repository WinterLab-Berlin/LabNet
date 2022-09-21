cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML COMPONENTS audio REQUIRED)
find_path(BOOST_SIGNALS2_INCLUDE_DIRS "boost/signals2.hpp")
find_path(BOOST_ASIO_INCLUDE_DIRS "boost/asio.hpp")

add_executable(LabNet LabNet.cpp
               log/easylogging++.cc log/easylogging++.h log/logging_facility.h log/standard_output_logger.h log/easy_logger.h log/logger_factory.h               
               network/connection.cpp network/connection.h network/connection_manager.cpp network/connection_manager.h network/server.cpp network/server.h
               network/LabNet.pb.h network/LabNetClient.pb.h network/LabNetServer.pb.h network/LabNet.pb.cc network/LabNetClient.pb.cc
               network/LabNetServer.pb.cc network/protocol_all.h network/shared_buffer.h
               network/server_messages.h network/server_actor.h network/server_actor.cpp
               interface/uart/serial_port.cpp interface/uart/serial_port.h  interface/uart/serial_ports_manager.h
               interface/uart/serial_ports_manager.cpp interface/uart/private_messages.h interface/uart/digital_input.h interface/uart/digital_output.h 
               interface/io_board/board_actor.h interface/io_board/board_actor.cpp interface/io_board/digital_input_state_reader.cpp
               interface/io_board/digital_input.h interface/io_board/digital_output.h interface/io_board/digital_input_state_reader.h
               interface/io_board/resource_request_helper.h interface/io_board/resource_request_helper.cpp
               interface/rfid_board/data_read_worker.h interface/rfid_board/max_14830.h
               interface/rfid_board/max_14830_defs.h interface/rfid_board/max_device.h interface/rfid_board/data_read_worker.cpp
               interface/rfid_board/max_14830.cpp interface/rfid_board/max_device.cpp interface/rfid_board/spi.h interface/rfid_board/rfid_main_actor.cpp
               interface/rfid_board/rfid_main_actor.h 
               interface/gpio_wiring/gpio_manager.h interface/gpio_wiring/digital_input.h interface/gpio_wiring/digital_output.h interface/gpio_wiring/digital_input_state_reader.h
               interface/gpio_wiring/digital_input_state_reader.cpp interface/gpio_wiring/gpio_manager.cpp
               interface/gpio_wiring/resource_request_helper.h interface/gpio_wiring/resource_request_helper.cpp
               interface/sound/signal_generator.h interface/sound/signal_generator.cpp interface/sound/signals.h
               interface/digital_messages.h interface/stream_messages.h
               interface/manage_interfaces.h interface/interface_messages.h
               interface/manage_interfaces.cpp interface/interfaces.h
               digital_out/digital_out_helper.h digital_out/digital_out_helper.cpp digital_out/loop_messages.h digital_out/pulse_helper.h
               digital_out/pulse_helper.cpp digital_out/loop_helper.h digital_out/pin_id.h digital_out/loop_helper.cpp
               helper/reset_helper.h helper/reset_helper.cpp helper/reset_msg.h
               resources/resource.h resources/resource.cpp resources/resources_actor.h  resources/resources_actor.cpp
               "interface/chi_bio/chi_bio_main_actor.h" "interface/chi_bio/chi_bio_main_actor.cpp"
               interface/ble_uart/ble_uart_main.h interface/ble_uart/ble_uart_main.cpp interface/ble_uart/connection.h interface/ble_uart/connection.cpp
               interface/ble_uart/ble_uart_actor.h interface/ble_uart/ble_uart_actor.cpp
               interface/uart_board/max_uart_board.h interface/uart_board/max_uart_board.cpp
               interface/uart_board/uart_board_main_actor.h interface/uart_board/uart_board_main_actor.cpp
               interface/uart_board/data_worker.h interface/uart_board/data_worker.cpp)
                

target_include_directories(LabNet PRIVATE ${BOOST_ASIO_INCLUDE_DIRS} ${BOOST_SIGNALS2_INCLUDE_DIRS})
target_include_directories(LabNet PRIVATE ../vendor/sobjectizer/dev)
target_include_directories(LabNet PRIVATE ../vendor/gattlib/include)
target_include_directories(LabNet PRIVATE log)

target_link_libraries(LabNet atomic pthread boost_system wiringPi so.5.7.3 protobuf sfml-system sfml-audio gattlib)

add_compile_definitions(ELPP_THREAD_SAFE)
add_compile_definitions(ELPP_NO_DEFAULT_LOG_FILE)
FILE(COPY log/logger.conf DESTINATION "${CMAKE_BINARY_DIR}")