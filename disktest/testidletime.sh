#!/bin/bash
# 调度算法更改位置
scheduler_path=/sys/block/sdc/queue/scheduler
checkfifo_path=/sys/block/sdc/queue/iosched/checkfifo
antic_expire_path=/sys/block/sdc/queue/iosched/antic_expire
thread_num=8
# 测试程序路径
test_proc=./test
# 测试文件地址
file_path=/disk0
# 生成文件前缀
prefix=olddisk

antic_expire=0

scheduler=vod_v4
echo $scheduler > $scheduler_path
echo 0 > $checkfifo_path

while [ $antic_expire -le 10 ]
do
	echo antic_expire > $antic_expire_path
	echo antic_expire >> ${prefix}_${scheduler}_10M.csv
	echo "," >> ${prefix}_${scheduler}_10M.csv
	$test_proc $file_path $thread_num 600 10485760 13 >> ${prefix}_${scheduler}_10M.csv
	let antic_expire+=1
	sleep 60
done
