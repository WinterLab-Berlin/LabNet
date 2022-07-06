# ping test

This test is done in two ways.
- the "id test" requests the ID of a LabNet server, without any operations to the peripheral hardware. It measures the time interval between sending the request and arrival of the answer;
- the "set out test" toggles a digital output is alternately set to 0 or 1. After the command for setting the pin has been received and processed, LabNet automatically sends back an acknowledgement. In this test, the time between sending the set command and receiving this confirmation was measured.

## perform the test
1. you need .NET 6 SDK;
2. open the project with Visual Studio or VS Code;
3. All missing packages should be installed automatically;
4. LabNet server on the RasPi has to be up and running;
5. change the ip address in "Program.cs" if necessary;
6. run the project.
