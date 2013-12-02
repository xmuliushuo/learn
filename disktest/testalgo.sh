#!/bin/bash
# 调度算法更改位置
scheduler_path=/sys/block/sdc/queue/scheduler
#checkfifo_path=/sys/block/sdc/queue/iosched/checkfifo
#antic_expire_path=/sys/block/sdc/queue/iosched/antic_expire
thread_num=1
# 测试程序路径
test_proc=./test
# 测试文件地址
file_path=/disk0
# 生成文件前缀
prefix=newdisk

#antic_expire=0

scheduler=noop
echo $scheduler > $scheduler_path
#echo 0 > $checkfifo_path

while [ $thread_num -le 15 ]
do
	$test_proc $file_path $thread_num 120 10485760 13 >> ${prefix}_${scheduler}_10M.csv
	let thread_num+=1
	sleep 60
done
