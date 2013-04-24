#!/bin/bash
echo "Vector size = 512"; ./bin/test -v 64 -u 1 -t 50 -s 512 -x 512 
echo "Vector size = 1024"; ./bin/test -v 64 -u 2 -t 50 -s 512 -x 512 
echo "Vector size = 2048"; ./bin/test -v 64 -u 3 -t 50 -s 512 -x 512 
echo "Vector size = 4096"; ./bin/test -v 64 -u 4 -t 50 -s 512 -x 512 
echo "Vector size = 8192"; ./bin/test -v 64 -u 5 -t 50 -s 512 -x 512 
echo "Vector size = 16384"; ./bin/test -v 64 -u 6 -t 50 -s 512 -x 512 
echo "Vector size = 32768"; ./bin/test -v 64 -u 7 -t 50 -s 512 -x 512 
