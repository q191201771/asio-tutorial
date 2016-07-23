#!/bin/bash

rm *.log.chef
ulimit -c unlimited
ulimit -n 65535
killall server3
nohup ./server3 5566 16 &
