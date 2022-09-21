# Build instructions

The project uses CMake as build system. We will describe two compilation ways. Both ways need a running Raspberry Pi. But in case of Visual Studio only for debugging.

## Requirements
On Raspberry Pi we need some packages.
1. CMake
2. libsfml-dev
3. libasio-dev
4. libboost-system-dev
5. wiringpi
6. libprotoc-dev
7. libbluetooth-dev
8. gdbserver - only for debugging with Visual Studio 2019.

## On RaspberryPi

We recomend to use [VS Code](https://code.visualstudio.com) as IDE. You will need following extensions: C/C++ and CMake Tools from Microsoft. If you want to use VS Code remotelly you will also need Remove-SSH extension from Microsoft.

We also recomment to use Raspberry Pi 4 with at least 2GB RAM otherwise the compilation can take a long time. 

1. Clone the repository with submodules:
    ```sh
    git clone --recursive https://github.com/WinterLab-Berlin/LabNet
    ```
2. Open the folder with VS Code.
3. Select as kit the GCC 8.3.0.
4. Select as build target Release.
5. Build the project.

## Visual Studio 2019
With Visual Studio 2019 it is also possible to compile and remotely debug applications for Linux. We need the "C++-CMake-Tools for Linux" component for this.

### Docker
To speedup the compilation we will use a docker image which will contain the complete toolchain.

We need "Toolchain-rpi.cmake" and "Dockerfile" files from doc folder. The toolchain is based on [raspi-toolchain](https://github.com/Pro/raspi-toolchain).

1. Change in the terminal to the folder with both files.
2. build the docker image
    ```sh
    docker build -t debian-vs -f .\Dockerfile .
    ```
3. start the new container
    ```sh
    docker run -d -p 5000:22 debian-vs
    ```
4. connect with ssh to the container with:
    ```sh
    ssh test@localhost -p 5000
    ```
    the password is also "test".
5. to cross-compile we also need all current libraries and include files from your raspberry:
    ```sh
    # Use the correct IP address here
    rsync -vR --progress -rl --delete-after --safe-links pi@192.168.1.PI:/{lib,usr,etc/ld.so.conf.d,opt/vc/lib} $HOME/rpi/rootfs
    ```
6. the docker container is now ready to use.

### Visual Studio 2019
Don't forget to install the "C++-CMake-Tools for Linux" component.

1. Clone the repository with submodules:
    ```sh
    git clone --recursive https://github.com/WinterLab-Berlin/LabNet
    ```
2. Open the LabNet folder with Visual Studio.
3. Configure both ssh connections to the docker container and the target Raspberry Pi under "Tools\Options\Cross Platform\Connection Manager". Select the Raspberry Pi connection as default one.
4. Open "CMakeSettings.json".
5. Choose under "Remote machine name" the docker container connection for Debug and Release configurations.
6. This step is only necessary if Raspberry Pi connection is not the default one. Right click on "CMakeList.txt" and select "Open Debug And Launch Settings". In the json file delete the value of the "remoteMachineName" parameter. Press "Ctr+Spacebar" and select the right connection.
7. Choose the build configuration "Linux-GCC-Release" or "Linux-GCC-Debug" and "gdbserver" as startup item.
8. If you press "Start Debugging" Visual Studio will compile LabNet on the docker container, transfer it to Raspberry Pi and start over gdbserver.