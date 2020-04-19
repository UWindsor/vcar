#!/bin/bash

modprobe vcan
ip link add dev vcar type vcan
ip link set up vcar
