#!/bin/bash

rm *.log.chef
ulimit -c unlimited
ulimit -n 65535
valgrind --leak-check=full ./client_benchmark 127.0.0.1 5566 512
