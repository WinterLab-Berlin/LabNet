# LabNet

LabNet is a server to control hardware connected to RaspberryPi over Ethernet in behavioral experiment with animals. The protocol uses Google [Protobuf](https://github.com/protocolbuffers/protobuf) for the TCP/IP transport. The main goals are:

1.	Simple communication protocol.
2.	Easy to add support for new hardware.
3.	Low latencies. Commands are mostly executed in 1ms, which includes also the communication over the network.

Build instructions can be found [here](doc/build_instructions.md).
