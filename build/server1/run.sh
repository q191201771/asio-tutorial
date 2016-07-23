#!/bin/bash

rm *.log.chef
ulimit -c unlimited
ulimit -n 65535
killall server1
nohup ./server1 5566 &
