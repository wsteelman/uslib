#!/bin/bash

for i in {1..100}
do
   rand=$RANDOM
   UP=`expr 2 + $rand % 9`
   rand=$RANDOM
   SAMPLE=`expr 3 + $rand % 10`
   SAMPLE=$( echo "2^$SAMPLE" | bc )
   #echo "L: $UP, S: $SAMPLE"  
   echo `./bin/test -v 256 -f 4 -u $UP -t 10 -s $SAMPLE -x $SAMPLE`
done
