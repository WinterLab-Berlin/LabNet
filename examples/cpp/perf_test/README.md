# Read and set test

This is a part of the "Read and set" and "stress" tests and needs to be started on the an own RasPi. For the "Read and set" two GPIOs has to be connected to RasPi with LabNet. For the stress test we need to connect up to 28 GPIOs.

The pins will be configurated so that the output pin is connected to the input of LabNet and input pin to the output on LabNet. This program sets the output to 1 and waits until the input pin is also set to 1 from the LabNet. The program measures the time between these two events. Then it turns the output pin off and repeat all steps after a short delay.

LabNet sends the input events to the client machine and the client has to send a message to turn the output pin on or off. This procedure allows to test how fast LabNet together with a client can react to external events.

The results are saved into "latencies.csv" file.

## parameter
1. for input and output pins, see arrays "in_pins" and "out_pins" in main.cpp.
2. the number of tests which are running in parallel, can be specified via the command line. max is 14.

## run the test
1. start LabNet on its own RasPi;
2. start "read_set_test" client from C#, Python or C++ examples folder;
3. start this program.

