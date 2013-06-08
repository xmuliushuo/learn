#!/bin/bash

filename=cfq_10M
threadnum=1

echo "cfq" >> $filename
echo "one read 10M" >> $filename
while [ $threadnum -le 20 ]
do
    echo $threadnum thread: >> $filename
    ./a.out /disk0 $threadnum 300 10485760 13 >> $filename
    let threadnum+=3
done
