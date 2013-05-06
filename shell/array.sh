#!/bin/bash

array_var1=(0 1 2 3 4 5 6)

array_var2[0]="test1"
array_var2[1]="test2"

echo ${array_var2[0]}
index=1
echo ${array_var2[$index]}

echo ${array_var2[*]}
echo ${array_var2[@]}
echo ${#array_var2[*]}

