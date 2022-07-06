# ping test

This test is done in two ways.
- the "id test" requests the ID of a LabNet server, without any operations to the peripheral hardware. It measures the time interval between sending the request and arrival of the answer;
- the "set out test" toggles a digital output is alternately set to 0 or 1. After the command for setting the pin has been received and processed, LabNet automatically sends back an acknowledgement. In this test, the time between sending the set command and receiving this confirmation was measured.

## perform the test
1. install all packages from "requirements.txt", best in a virtual environment;
2. LabNet server on the RasPi has to be up and running;
3. change the "labnet_ip" in test.py to the ip address of your RasPi;
4. start test.py