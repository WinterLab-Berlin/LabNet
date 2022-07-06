import time
import numpy as np

import LabNet_pb2 as LabNet
import LabNetClient_pb2 as LabNetClient
import LabNetServer_pb2 as LabNetServer

from client import Client

labnet_ip = "192.168.137.101"
cycles = 10_000
id_latencies = []
set_latencies = []

with Client() as labNet:
    print("start ID test")
    labNet.connect(labnet_ip, 8080)
    
    reset = LabNetClient.LabNetResetRequest()
    labNet.send(reset)
    reply = labNet.read()
    if not isinstance(reply, LabNetServer.LabNetResetReply):
        print("wrong response to reset request")
        exit()

    req = LabNetClient.LabNetIdRequest()
    
    for r in range(cycles):
        print(f"\r{r+1} from {cycles}", end='', flush=True)
        t0 = time.perf_counter_ns()
        labNet.send(req)
        
        mes = labNet.read()
        if isinstance(mes, LabNetServer.LabNetIdReply):
            el = (time.perf_counter_ns() - t0) / 1_000_000
            id_latencies.append(el)
        else:
            print("wrong response")
    
    median = np.percentile(id_latencies, 50)
    p2_5 = np.percentile(id_latencies, 2.5)
    p25 = np.percentile(id_latencies, 25)
    p75 = np.percentile(id_latencies, 75)
    p97_5 = np.percentile(id_latencies, 97.5)
    mean = np.mean(id_latencies)
    std = np.std(id_latencies)

    print(f"\rmean: {mean:.2f} std dev: {std:.2f} median: {median:.2f} p25: {p25:.2f} p75: {p75:.2f} p2.5: {p2_5:.2f} p97.5: {p97_5:.2f}")


with Client() as labNet:
    print("start set digital out test")
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

    turn_on = True
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
            set_latencies.append(el)
        else:
            print("wrong response")
        
        turn_on = not turn_on

    median = np.percentile(set_latencies, 50)
    p2_5 = np.percentile(set_latencies, 2.5)
    p25 = np.percentile(set_latencies, 25)
    p75 = np.percentile(set_latencies, 75)
    p97_5 = np.percentile(set_latencies, 97.5)
    mean = np.mean(set_latencies)
    std = np.std(set_latencies)

    print(f"\rmean: {mean:.2f} std dev: {std:.2f} median: {median:.2f} p25: {p25:.2f} p75: {p75:.2f} p2.5: {p2_5:.2f} p97.5: {p97_5:.2f}")


print("save latencies")
f = open("latencies.csv", "w")
f.write("IdTest;SetTest\n")
for r in range(cycles):
    f.write(f"{id_latencies[r]:.2f};{set_latencies[r]:.2f}\n")

print("save latencies done")