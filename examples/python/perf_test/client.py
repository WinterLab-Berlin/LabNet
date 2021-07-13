import socket
import struct

import LabNet_pb2 as LabNet
import LabNetClient_pb2 as LabNetClient
import LabNetServer_pb2 as LabNetServer

from google.protobuf.internal.encoder import _VarintBytes
from google.protobuf.internal.decoder import _DecodeVarint32
import google.protobuf.message as gpm

class Client:

    def __init__(self, sock=None):
        if sock is None:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        else:
            self.sock = sock
        self.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, True)

    def __enter__(self, sock=None):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.sock.close()

    def connect(self, host, port):
        self.sock.connect((host, port))

    def send(self, data):
        mes_id = -1
        if isinstance(data, LabNetClient.LabNetIdRequest):
            mes_id = LabNetClient.LABNET_ID_REQUEST
        elif isinstance(data, LabNetClient.LabNetResetRequest):
            mes_id = LabNetClient.LABNET_RESET_REQUEST
        elif isinstance(data, LabNetClient.IoBoardInit):
            mes_id = LabNetClient.IO_BOARD_INIT
        elif isinstance(data, LabNetClient.IoBoardInitDigitalIn):
            mes_id = LabNetClient.IO_BOARD_INIT_DIGITAL_IN
        elif isinstance(data, LabNetClient.IoBoardInitDigitalOut):
            mes_id = LabNetClient.IO_BOARD_INIT_DIGITAL_OUT
        elif isinstance(data, LabNetClient.RfidBoardInit):
            mes_id = LabNetClient.RFID_BOARD_INIT
        elif isinstance(data, LabNetClient.RfidBoardSetPhaseMatrix):
            mes_id = LabNetClient.RFID_BOARD_SET_PHASE_MATRIX
        elif isinstance(data, LabNetClient.UartInit):
            mes_id = LabNetClient.UART_INIT
        elif isinstance(data, LabNetClient.UartWriteData):
            mes_id = LabNetClient.UART_WRITE_DATA
        elif isinstance(data, LabNetClient.DigitalOutSet):
            mes_id = LabNetClient.DIGITAL_OUT_SET
        elif isinstance(data, LabNetClient.DigitalOutPulse):
            mes_id = LabNetClient.DIGITAL_OUT_PULSE
        elif isinstance(data, LabNetClient.StartDigitalOutLoop):
            mes_id = LabNetClient.START_DIGITAL_OUT_LOOP
        elif isinstance(data, LabNetClient.StopDigitalOutLoop):
            mes_id = LabNetClient.STOP_DIGITAL_OUT_LOOP
        elif isinstance(data, LabNetClient.GpioWiringPiInit):
            mes_id = LabNetClient.GPIO_WIRINGPI_INIT
        elif isinstance(data, LabNetClient.GpioWiringPiInitDigitalIn):
            mes_id = LabNetClient.GPIO_WIRINGPI_INIT_DIGITAL_IN
        elif isinstance(data, LabNetClient.GpioWiringPiInitDigitalOut):
            mes_id = LabNetClient.GPIO_WIRINGPI_INIT_DIGITAL_OUT
        elif isinstance(data, LabNetClient.InitSound):
            mes_id = LabNetClient.INIT_SOUND
        elif isinstance(data, LabNetClient.InitSoundSignal):
            mes_id = LabNetClient.INIT_SOUND_SIGNAL
        elif isinstance(data, LabNetClient.UartInitDigitalIn):
            mes_id = LabNetClient.UART_INIT_DIGITAL_IN
        elif isinstance(data, LabNetClient.UartInitDigitalOut):
            mes_id = LabNetClient.UART_INIT_DIGITAL_OUT

        if mes_id > 0:
            self.sock.send(_VarintBytes(mes_id))
            self.sock.send(_VarintBytes(data.ByteSize()))
            self.sock.send(data.SerializeToString())

    def read(self):
        msg_type = 0
        shift = 0
        while True:
            vbyte, = struct.unpack('b', self.sock.recv(1))
            msg_type += (vbyte & 0x7f) << shift
            shift += 7
            if not vbyte & 0x80:
                break

        shift = 0
        msg_size = 0
        while True:
            vbyte, = struct.unpack('b', self.sock.recv(1))
            msg_size += (vbyte & 0x7f) << shift
            shift += 7
            if not vbyte & 0x80:
                break

        data = []
        while msg_size:
            buf = self.sock.recv(msg_size)
            if not buf:
                raise ValueError("Buffer receive truncated")
            data.append(buf)
            msg_size -= len(buf)

        msg = None
        if msg_type == LabNetServer.LABNET_ID_REPLY:
            msg = LabNetServer.LabNetIdReply()
        elif msg_type == LabNetServer.LABNET_RESET_REPLY:
            msg = LabNetServer.LabNetResetReply()
        elif msg_type == LabNetServer.DIGITAL_OUT_STATE:
            msg = LabNetServer.DigitalOutState()
        elif msg_type == LabNetServer.DIGITAL_IN_STATE:
            msg = LabNetServer.DigitalInState()
        elif msg_type == LabNetServer.NEW_BYTE_DATA:
            msg = LabNetServer.NewByteData()
        elif msg_type == LabNetServer.DATA_WRITE_COMPLETE:
            msg = LabNetServer.DataWriteComplete()
        elif msg_type == LabNetServer.INTERFACE_INIT_RESULT:
            msg = LabNetServer.InterfaceInitResult()
        elif msg_type == LabNetServer.DIGITAL_IN_INIT_RESULT:
            msg = LabNetServer.DigitalInInitResult()
        elif msg_type == LabNetServer.DIGITAL_OUT_INIT_RESULT:
            msg = LabNetServer.DigitalOutInitResult()
        elif msg_type == LabNetServer.ONLY_ONE_CONNECTION_ALLOWED:
            msg = LabNetServer.OnlyOneConnectionAllowed()
        elif msg_type == LabNetServer.INTERFACE_LOST:
            msg = LabNetServer.InterfaceLost()
        elif msg_type == LabNetServer.INTERFACE_RECONNECTED:
            msg = LabNetServer.InterfaceReconnected()
        elif msg_type == LabNetServer.DIGITAL_OUT_LOOP_START_RESULT:
            msg = LabNetServer.DigitalOutLoopStartResult()
        elif msg_type == LabNetServer.DIGITAL_OUT_LOOP_STOPPED:
            msg = LabNetServer.DigitalOutLoopStopped()

        if msg:
            msg.ParseFromString(b''.join(data))
            return msg
        else:
            return None
