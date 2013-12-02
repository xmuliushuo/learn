#!/bin/bash
# 调度算法更改位置
scheduler_path=/sys/block/sdc/queue/scheduler
checkfifo_path=/sys/block/sdc/queue/iosched/checkfifo
back_max_path=/sys/block/sdc/queue/iosched/back_max
thread_num=8
# 测试程序路径
test_proc=./test
# 测试文件地址
file_path=/disk0
# 生成文件前缀
prefix=olddisk

back_max=0

scheduler=vod_v4
echo $scheduler > $scheduler_path
echo 0 > $checkfifo_path

echo $back_max > $back_max_path
echo -e "${back_max},\c" >> ${prefix}_${scheduler}_10M.csv
$test_proc $file_path $thread_num 600 10485760 13 >> ${prefix}_${scheduler}_10M.csv
sleep 60

back_max=1048576
echo $back_max > $back_max_path
echo -e "${back_max},\c" >> ${prefix}_${scheduler}_10M.csv
$test_proc $file_path $thread_num 600 10485760 13 >> ${prefix}_${scheduler}_10M.csv
sleep 60

back_max=10485760
echo $back_max > $back_max_path
echo -e "${back_max},\c" >> ${prefix}_${scheduler}_10M.csv
$test_proc $file_path $thread_num 600 10485760 13 >> ${prefix}_${scheduler}_10M.csv
sleep 60

back_max=104857600
echo $back_max > $back_max_path
echo -e "${back_max},\c" >> ${prefix}_${scheduler}_10M.csv
$test_proc $file_path $thread_num 600 10485760 13 >> ${prefix}_${scheduler}_10M.csv
sleep 60

back_max=1048576000
echo $back_max > $back_max_path
echo -e "${back_max},\c" >> ${prefix}_${scheduler}_10M.csv
$test_proc $file_path $thread_num 600 10485760 13 >> ${prefix}_${scheduler}_10M.csv
sleep 60