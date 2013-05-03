#!/bin/bash
echo "Welcome to bash"
echo Welcome to bash
echo 'Welcome to bash'

# We cannot use echo hello;hello
var="value"
echo $var   #value
echo '$var' #$var
echo "$var" #value

printf "%-5s %-10s %-4s\n" No Name Mark
printf "%-5s %-10s %-4.2f\n" 1 Sarath 80.3456
