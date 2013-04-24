for UP in {2..10}
do
   SAMPLE=8
   while [[ $SAMPLE -le 4096 ]]
   do  
      echo `./bin/test -v 256 -f 4 -u $UP -t 10 -s $SAMPLE -x $SAMPLE`
      SAMPLE=`expr $SAMPLE * 2`
   done
done
