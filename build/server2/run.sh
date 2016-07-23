#!/bin/bash

rm *.log.chef
ulimit -c unlimited
ulimit -n 65535
killall server2
nohup ./server2 5566 16 &
