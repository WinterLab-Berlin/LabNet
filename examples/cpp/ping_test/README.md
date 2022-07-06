# ping test

This test is done in two ways.
- the "id test" requests the ID of a LabNet server, without any operations to the peripheral hardware. It measures the time interval between sending the request and arrival of the answer;
- the "set out test" toggles a digital output is alternately set to 0 or 1. After the command for setting the pin has been received and processed, LabNet automatically sends back an acknowledgement. In this test, the time between sending the set command and receiving this confirmation was measured.

## perform the test
1. clone sobjectizer with "git clone https://github.com/Stiffstream/sobjectizer.git" to this folder;
2. correct the sobjectizer library name in "CMakeLists.txt" in "target_link_libraries";
3. change the ip address in "main.cpp";
4. build the project;
5. start LabNet server on the RasPi;
6. execute the project.