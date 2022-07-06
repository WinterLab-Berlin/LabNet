import time
import numpy as np

import LabNet_pb2 as LabNet
import LabNetClient_pb2 as LabNetClient
import LabNetServer_pb2 as LabNetServer

from client import Client

labnet_ip = "192.168.137.101"

in_pins = [29, 28, 27, 26, 31, 11, 10, 6, 5, 4, 1, 16, 15, 8]
out_pins = [25, 24, 23, 22, 21, 30, 14, 13, 12, 3, 2, 0, 7, 9]
in_out_dict = dict(zip(in_pins, out_pins))


with Client() as labNet:
    print("start read and set GPIO test")
    labNet.connect(labnet_ip, 8080)

    reset = LabNetClient.LabNetResetRequest()
    labNet.send(reset)
    reply = labNet.read()
    if not isinstance(reply, LabNetServer.LabNetResetReply):
        print("wrong response to reset request")
        exit()

    init = LabNetClient.GpioWiringPiInit()
    labNet.send(init)
    reply = labNet.read()
    if not isinstance(reply, LabNetServer.InterfaceInitResult):
        print("wrong response to WiringPi init request")
        exit()
    elif not reply.is_succeed:
        print("cannot init WiringPi")
        exit()

    pin_init_nbr = 0
    while pin_init_nbr < len(out_pins):
        init = LabNetClient.GpioWiringPiInitDigitalOut()
        init.pin = out_pins[pin_init_nbr]
        init.is_inverted = False
        labNet.send(init)
        pin_init_nbr += 1

        reply = labNet.read()
        if not isinstance(reply, LabNetServer.DigitalOutInitResult):
            print("wrong response to digital out init request")
            exit()
        elif not reply.is_succeed:
            print("cannot init digital out")
            exit()
    
    pin_init_nbr = 0
    while pin_init_nbr < len(in_pins):
        init = LabNetClient.GpioWiringPiInitDigitalIn()
        init.pin = in_pins[pin_init_nbr]
        init.is_inverted = False
        labNet.send(init)
        pin_init_nbr += 1

        reply = labNet.read()
        while not isinstance(reply, LabNetServer.DigitalInInitResult):
            reply = labNet.read()
        if not reply.is_succeed:
            print("cannot init digital in")
            exit()

    while True:
        mes = labNet.read()
        if isinstance(mes, LabNetServer.DigitalInState):
            pin = mes.pin.pin
            if pin in in_out_dict:
                if mes.state:
                    set_on_cmd = LabNetClient.DigitalOutSet()
                    set_on_cmd.id.interface = LabNet.INTERFACE_GPIO_WIRINGPI
                    set_on_cmd.id.pin = in_out_dict[pin]
                    set_on_cmd.state = True
                    labNet.send(set_on_cmd)
                else:
                    set_off_cmd = LabNetClient.DigitalOutSet()
                    set_off_cmd.id.interface = LabNet.INTERFACE_GPIO_WIRINGPI
                    set_off_cmd.id.pin = in_out_dict[pin]
                    set_off_cmd.state = False
                    labNet.send(set_off_cmd)
