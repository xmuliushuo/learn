#!/bin/bash
no1=4
no2=5
let result=no1+no2
echo result: $result
let no1++
echo no1: $no1
let no2--
echo no2: $no2
let no1+=6
echo no1: $no1

result=$[no1+no2]
echo result: $result
result=$[$no1+5]
echo result: $result

