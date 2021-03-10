import time
import numpy as np

import LabNet_pb2 as LabNet
import LabNetClient_pb2 as LabNetClient
import LabNetServer_pb2 as LabNetServer

from client import Client


with Client() as labNet:
    print("start ID test")
    labNet.connect("192.168.137.101", 8080)
    
    reset = LabNetClient.LabNetResetRequest()
    labNet.send(reset)
    reply = labNet.read()
    if not isinstance(reply, LabNetServer.LabNetResetReply):
        print("wrong response to reset request")
        exit()

    req = LabNetClient.LabNetIdRequest()
    
    times = []
    cycles = 10_000
    for r in range(cycles):
        print(f"\r{r+1} from {cycles}", end='', flush=True)
        t0 = time.perf_counter_ns()
        labNet.send(req)
        
        mes = labNet.read()
        if isinstance(mes, LabNetServer.LabNetIdReply):
            el = (time.perf_counter_ns() - t0) / 1_000_000
            times.append(el)
        else:
            print("wrong response")
    
    median = np.percentile(times, 50)
    p95 = np.percentile(times, 95)
    min = np.min(times)
    max = np.max(times)
    mean = np.mean(times)
    std = np.std(times)

    print(f"\nmean: {mean:.2f} std dev: {std:.2f} median: {median:.2f} p95: {p95:.2f} min: {min:.2f} max: {max:.2f}")


with Client() as labNet:
    print("start set digital out test")
    labNet.connect("192.168.137.101", 8080)

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

    init = LabNetClient.GpioWiringPiInitDigitalOut()
    init.pin = 5
    init.is_inverted = False
    labNet.send(init)
    reply = labNet.read()
    if not isinstance(reply, LabNetServer.DigitalOutInitResult):
        print("wrong response to digital out init request")
        exit()
    elif not reply.is_succeed:
        print("cannot init digital out")
        exit()

    set_on_cmd = LabNetClient.DigitalOutSet()
    set_on_cmd.id.interface = LabNet.INTERFACE_GPIO_WIRINGPI
    set_on_cmd.id.pin = 5
    set_on_cmd.state = True

    set_off_cmd = LabNetClient.DigitalOutSet()
    set_off_cmd.id.interface = LabNet.INTERFACE_GPIO_WIRINGPI
    set_off_cmd.id.pin = 5
    set_off_cmd.state = False

    times = []
    turn_on = True
    cycles = 10_000
    for r in range(cycles):
        print(f"\r{r+1} from {cycles}", end='', flush=True)
        t0 = time.perf_counter_ns()
        if turn_on:
            labNet.send(set_on_cmd)
        else:
            labNet.send(set_off_cmd)

        mes = labNet.read()
        if isinstance(mes, LabNetServer.DigitalOutState) and mes.state == turn_on:
            el = (time.perf_counter_ns() - t0) / 1_000_000
            times.append(el)
        else:
            print("wrong response")
        
        turn_on = not turn_on

    median = np.percentile(times, 50)
    p95 = np.percentile(times, 95)
    min = np.min(times)
    max = np.max(times)
    mean = np.mean(times)
    std = np.std(times)

    print(
        f"\nmean: {mean:.2f} std dev: {std:.2f} median: {median:.2f} p95: {p95:.2f} min: {min:.2f} max: {max:.2f}")


with Client() as labNet:
    print("start set and read GPIO test")
    labNet.connect("192.168.137.101", 8080)

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

    init = LabNetClient.GpioWiringPiInitDigitalOut()
    init.pin = 5
    init.is_inverted = False
    labNet.send(init)
    reply = labNet.read()
    if not isinstance(reply, LabNetServer.DigitalOutInitResult):
        print("wrong response to digital out init request")
        exit()
    elif not reply.is_succeed:
        print("cannot init digital out")
        exit()
    
    init = LabNetClient.GpioWiringPiInitDigitalIn()
    init.pin = 23
    init.is_inverted = False
    labNet.send(init)
    reply = labNet.read()
    if not isinstance(reply, LabNetServer.DigitalInInitResult):
        print("wrong response to digital in init request")
        exit()
    elif not reply.is_succeed:
        print("cannot init digital in")
        exit()

    dig_in_state = labNet.read()
    if not isinstance(dig_in_state, LabNetServer.DigitalInState):
        print("wrong response to digital in init request")
        exit()

    set_on_cmd = LabNetClient.DigitalOutSet()
    set_on_cmd.id.interface = LabNet.INTERFACE_GPIO_WIRINGPI
    set_on_cmd.id.pin = 5
    set_on_cmd.state = True

    set_off_cmd = LabNetClient.DigitalOutSet()
    set_off_cmd.id.interface = LabNet.INTERFACE_GPIO_WIRINGPI
    set_off_cmd.id.pin = 5
    set_off_cmd.state = False

    times = []
    turn_on = not dig_in_state.state
    cycles = 10_000
    for r in range(cycles):
        print(f"\r{r+1} from {cycles}", end='', flush=True)
        t0 = time.perf_counter_ns()
        if turn_on:
            labNet.send(set_on_cmd)
        else:
            labNet.send(set_off_cmd)

        while True:
            mes = labNet.read()
            if isinstance(mes, LabNetServer.DigitalInState) and mes.state == turn_on:
                el = (time.perf_counter_ns() - t0) / 1_000_000
                times.append(el)
                break

        turn_on = not turn_on

    median = np.percentile(times, 50)
    p95 = np.percentile(times, 95)
    min = np.min(times)
    max = np.max(times)
    mean = np.mean(times)
    std = np.std(times)

    print(
        f"\nmean: {mean:.2f} std dev: {std:.2f} median: {median:.2f} p95: {p95:.2f} min: {min:.2f} max: {max:.2f}")
