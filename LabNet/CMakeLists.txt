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
               Interface/uart/SerialPort.cpp Interface/uart/SerialPort.h  Interface/uart/SerialPortsManager.h
               Interface/uart/SerialPortsManager.cpp
               Interface/uart/PrivateMessages.h Interface/Interfaces.h
               Interface/io_board/GPIOManager.h Interface/io_board/GPIOManager.cpp  Interface/io_board/DigitalInputStateReader.cpp
               Interface/io_board/DigitalInput.h Interface/io_board/DigitalOutput.h Interface/io_board/DigitalInputStateReader.h
               Interface/rfid_board/DataReadWorker.h Interface/rfid_board/MAX14830.h
               Interface/rfid_board/MAX14830_defs.h Interface/rfid_board/MAXDevice.h Interface/rfid_board/spi.h Interface/rfid_board/DataReadWorker.cpp
               Interface/rfid_board/MAX14830.cpp Interface/rfid_board/MAXDevice.cpp Interface/rfid_board/spi.cpp Interface/rfid_board/RfidMainActor.cpp
               Interface/rfid_board/RfidMainActor.h 
               Interface/gpio_wiring/GpioManager.h Interface/gpio_wiring/DigitalInput.h Interface/gpio_wiring/DigitalOutput.h Interface/gpio_wiring/DigitalInputStateReader.h
               Interface/gpio_wiring/DigitalInputStateReader.cpp Interface/gpio_wiring/GpioManager.cpp
               Interface/sound/SignalGenerator.h Interface/sound/SignalGenerator.cpp Interface/sound/Signals.h
               Interface/DigitalMessages.h Interface/StreamMessages.h
               Interface/ManageInterfaces.h Interface/InterfaceMessages.h
               Interface/ManageInterfaces.cpp
               DigitalOut/DigitalOutHelper.h DigitalOut/DigitalOutHelper.cpp DigitalOut/LoopMessages.h DigitalOut/PulseHelper.h
               DigitalOut/PulseHelper.cpp DigitalOut/LoopHelper.h DigitalOut/PinId.h DigitalOut/LoopHelper.cpp)

target_include_directories(LabNet PRIVATE ${BOOST_ASIO_INCLUDE_DIRS} ${BOOST_SIGNALS2_INCLUDE_DIRS})
target_include_directories(LabNet PRIVATE ../sobjectizer/dev)
target_include_directories(LabNet PRIVATE Log)

target_link_libraries(LabNet atomic pthread boost_system wiringPi so.5.7.1 protobuf sfml-system sfml-audio)

add_compile_definitions(ELPP_THREAD_SAFE)
add_compile_definitions(ELPP_NO_DEFAULT_LOG_FILE)
FILE(COPY Log/logger.conf DESTINATION "${CMAKE_BINARY_DIR}")