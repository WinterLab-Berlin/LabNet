# Read and set test

This is a part of the "Read and set" and of the "stress" tests and needs to be started on the second RasPi. For the "Read and set" two GPIOs has to be crossover connected to the first RasPi. For the stress test we need to connect up to 28 GPIOs.

"perf_test" will toggle a output pin which is also a input for this program. This program will receive digital input events from LabNet and has to toggle the right output pin as reaction. "perf_test" will measure the time between the two pin changes.

## parameter
1. for input and output pins, see arrays "in_pins" and "out_pins" in read_and_set_test.cpp.
2. LabNet ip address is in the main.cpp.

## perform the test
1. start LabNet on the first RasPi;
2. start this program.
3. start "perf_test" from C++ examples folder.
