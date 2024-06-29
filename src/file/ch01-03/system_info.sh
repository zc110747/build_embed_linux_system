#!/bin/bash

local_time=$(date +"%Y%m%d %H:%M:%S")
local_ip=$(ifconfig eth0 | grep netmask | tr -s " " | cut -d" " -f3)
free_mem=$(cat /proc/meminfo | grep MemFree | tr -s " " | cut -d" " -f2)
free_disk=$(df | grep "/$" | tr -s " " | cut -d " " -f4)
cpu_loading=$(cat /proc/loadavg | cut -d" " -f3)
login_user=$(who | cut -d" " -f1)
procs=$(ps aux | wc -l)
irq=$(vmstat 1 2 | tail -n +4 | tr -s " " | cut -d " " -f12)
cs=$(vmstat 1 2 | tail -n +4 | tr -s " " | cut -d " " -f13)
usertime=$(vmstat 1 2 | tail -n +4 | tr -s " " | cut -d " " -f14)
systime=$(vmstat 1 2 | tail -n +4 | tr -s " " | cut -d " " -f15)
iowait=$(vmstat 1 2 | tail -n +4 | tr -s " " | cut -d " " -f17)
