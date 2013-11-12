#!/bin/bash
# 调度算法更改位置
scheduler_path=/sys/block/sdc/queue/scheduler
checkfifo_path=/sys/block/sdc/queue/iosched/checkfifo
# 线程个数
thread_num=1
# 测试程序路径
test_proc=./test
# 测试文件地址
file_path=/disk0
# 生成文件前缀
prefix=olddisk
# 最多多少个线程
max_thread=15

scheduler=vod_v4
echo $scheduler > $scheduler_path
echo 0 > $checkfifo_path

thread_num=1
while [ $thread_num -le $max_thread ]
do
	$test_proc $file_path $thread_num 120 10485760 13 >> ${prefix}_${scheduler}_10M.csv
	let thread_num+=1
	sleep 60
done

scheduler=cfq
echo $scheduler > $scheduler_path
thread_num=1
while [ $thread_num -le $max_thread ]
do
	$test_proc $file_path $thread_num 120 10485760 13 >> ${prefix}_${scheduler}_10M.csv
	let thread_num+=1
	sleep 60
done

scheduler=deadline
echo $scheduler > $scheduler_path
thread_num=1
while [ $thread_num -le $max_thread ]
do
	$test_proc $file_path $thread_num 120 10485760 13 >> ${prefix}_${scheduler}_10M.csv
	let thread_num+=1
	sleep 60
done

scheduler=anticipatory
echo $scheduler > $scheduler_path
thread_num=1
while [ $thread_num -le $max_thread ]
do
	$test_proc $file_path $thread_num 120 10485760 13 >> ${prefix}_${scheduler}_10M.csv
	let thread_num+=1
	sleep 60
done

scheduler=noop
echo $scheduler > $scheduler_path
thread_num=1
while [ $thread_num -le $max_thread ]
do
	$test_proc $file_path $thread_num 120 10485760 13 >> ${prefix}_${scheduler}_10M.csv
	let thread_num+=1
	sleep 60
done
