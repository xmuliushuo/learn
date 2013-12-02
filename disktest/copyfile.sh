#!/bin/bash

start=0
end=30000

sourcefile=file

while [ $start -l $end ]
do
	cp $sourcefile $start
	let start+=1
done