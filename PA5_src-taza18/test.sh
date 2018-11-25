#!/bin/sh

N=10000
buffer=(100 200)
workers=(100 200 300 400 500)

for buffersz in "${buffer[@]}"
do
  for worker in "${workers[@]}"
    do
      # echo "./client -n $N -w $worker -b $buffersz"
      seconds=$(./client -n $N -w $worker -b $buffersz | grep Took | awk '{print $2}')
      echo "b=$buffersz, w=$worker: $seconds"

      # clean up FIFOs if needed
      rm -rf fifo*
    done
done
