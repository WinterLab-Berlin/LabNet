# our local base image
FROM debian:buster

LABEL description="Container for use with Visual Studio" 

# install build dependencies 
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y sudo rsync zip openssh-server make wget cmake ninja-build pkg-config libglib2.0-dev-bin

RUN useradd -rm -d /home/dev -s /bin/bash -g root -G sudo -u 1000 test 

RUN  echo 'test:test' | chpasswd

RUN mkdir /home/dev/rpi
RUN wget https://github.com/Pro/raspi-toolchain/releases/latest/download/raspi-toolchain.tar.gz -P /home/dev/rpi/
RUN tar xfz /home/dev/rpi/raspi-toolchain.tar.gz --strip-components=1 -C /opt
RUN rm /home/dev/rpi/raspi-toolchain.tar.gz
COPY Toolchain-rpi.cmake /home/dev/rpi
RUN chown test /home/dev/rpi

RUN service ssh start

EXPOSE 22

CMD ["/usr/sbin/sshd","-D"]