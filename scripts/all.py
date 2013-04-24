#!/usr/bin/python

from subprocess import call

for UP in range(2,11):
   SAMPLE = 64
   while SAMPLE < 4097:
      call(["./bin/test", "-v", "256", "-f", "1", "-t", "10", "-u", str(UP), "-s", str(SAMPLE), "-x", str(SAMPLE)])
      SAMPLE = SAMPLE * 2

