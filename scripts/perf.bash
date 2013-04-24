#!/bin/bash
./bin/test -v     256 -f 1 -u 4 -t 25 -s 8  -x 8 
./bin/test -v    256 -f 1 -u 4 -t 25 -s 16 -x 16 
./bin/test -v    256 -f 1 -u 4 -t 25 -s 32 -x 32 
./bin/test -v    256 -f 1 -u 4 -t 25 -s 64 -x 64 
./bin/test -v   256 -f 1 -u 4 -t 25 -s 128 -x 128 
./bin/test -v   256 -f 1 -u 4 -t 25 -s 256 -x 256
./bin/test -v   256 -f 1 -u 4 -t 25 -s 512 -x 512
./bin/test -v  256 -f 1 -u 4 -t 25 -s 1024 -x 1024
./bin/test -v  256 -f 1 -u 4 -t 25 -s 2048 -x 2048
./bin/test -v  256 -f 1 -u 4 -t 25 -s 4096 -x 4096 
#echo "Samples = 8192"; ./bin/test -v  256 -f 1 -u 4 -t 25 -s 8192 -x 8192 
#echo "Samples = 16384"; ./bin/test -v 256 -f 1 -u 4 -t 25 -s 16384 -x 16384 
#echo "Samples = 32768"; ./bin/test -v 256 -f 1 -u 4 -t 25 -s 32768 -x 32768 
