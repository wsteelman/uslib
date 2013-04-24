#!/bin/bash
#echo "Upsample = 2"; ./bin/test -v 256 -u 2 -t 5 -s 2048 -x 2048 
#echo "Upsample = 3"; ./bin/test -v 256 -u 3 -t 5 -s 2048 -x 2048 
#echo "Upsample = 4"; ./bin/test -v 256 -u 4 -t 5 -s 2048 -x 2048 
#echo "Upsample = 5"; ./bin/test -v 256 -u 5 -t 5 -s 2048 -x 2048 
#echo "Upsample = 6"; ./bin/test -v 256 -u 6 -t 5 -s 2048 -x 2048 
#echo "Upsample = 7"; ./bin/test -v 256 -u 7 -t 5 -s 2048 -x 2048 
#echo "Upsample = 8"; ./bin/test -v 256 -u 8 -t 5 -s 2048 -x 2048 
#echo "Upsample = 9"; ./bin/test -v 256 -u 9 -t 5 -s 2048 -x 2048 
#echo "Upsample = 10"; ./bin/test -v 256 -u 10 -t 5 -s 2048 -x 2048
#echo "--------------------------------------------"
echo "Samples = 64"; ./bin/test -v 256 -u 4 -t 5 -s 64 -x 64 
echo "Samples = 128"; ./bin/test -v 256 -u 4 -t 5 -s 128 -x 128 
echo "Samples = 256"; ./bin/test -v 256 -u 4 -t 5 -s 256 -x 256
echo "Samples = 512"; ./bin/test -v 256 -u 4 -t 5 -s 512 -x 512
echo "Samples = 1024"; ./bin/test -v 256 -u 4 -t 5 -s 1024 -x 1024
echo "Samples = 2048"; ./bin/test -v 256 -u 4 -t 5 -s 2048 -x 2048
echo "Samples = 4096"; ./bin/test -v 256 -u 4 -t 5 -s 4096 -x 4096 
echo "Samples = 8192"; ./bin/test -v 256 -u 4 -t 5 -s 8192 -x 8192 
echo "Samples = 16384"; ./bin/test -v 256 -u 4 -t 5 -s 16384 -x 16384 
echo "Samples = 32768"; ./bin/test -v 256 -u 4 -t 5 -s 32768 -x 32768 
