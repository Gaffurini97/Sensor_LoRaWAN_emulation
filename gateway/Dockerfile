FROM ubuntu:20.04

RUN apt-get -y update
RUN apt-get -y upgrade

RUN apt-get -y install build-essential
RUN apt-get -y update
RUN apt-get -y install mosquitto
RUN apt-get -y install libmosquitto-dev
RUN apt-get -y update

RUN apt-get -y install make

COPY to-copy/gateway-v1.0.1 ./gateway-v1.0.1

WORKDIR gateway-v1.0.1

RUN make
