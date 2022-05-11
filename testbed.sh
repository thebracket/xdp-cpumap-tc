#!/bin/bash

# This script is designed to test on my local VM.
# You should be able to modify it as needed.
# Note that on my test VM, I had to update to 5.16 Linux.

TEST_INTERFACE="ens160"

# Set tuning parameters
ethtool --offload $TEST_INTERFACE gso off tso off lro off sg off gro off
ethtool -K $TEST_INTERFACE rxvlan off
sysctl net.core.bpf_jit_enable=1

# Setup XDP
bin/xps_setup.sh -d $TEST_INTERFACE --default --disable
src/xdp_iphash_to_cpu --dev $TEST_INTERFACE --lan
src/xdp_iphash_to_cpu_cmdline --clear
src/tc_classify --dev-egress $TEST_INTERFACE

# Clear queues
tc filter delete dev $TEST_INTERFACE
tc filter delete dev $TEST_INTERFACE root
tc qdisc delete dev $TEST_INTERFACE root
tc qdisc delete dev $TEST_INTERFACE

# Setup Master Multiqueues
tc qdisc replace dev $TEST_INTERFACE root handle 7FFF: mq
tc qdisc add dev $TEST_INTERFACE parent 7FFF:1 handle 1 htb default 2
tc class add dev $TEST_INTERFACE parent 1: htb rate 25mbit ceil 100mbit
tc qdisc add dev $TEST_INTERFACE parent 1:1 cake diffserv4
tc class add dev $TEST_INTERFACE parent 1:1 classid 1:2 htb rate 25mbit ceil 100mbit prio 5
tc qdisc add dev $TEST_INTERFACE parent 1:2 cake diffserv4

# Setup a test that should catch this computers IP in a queue
tc class add dev $TEST_INTERFACE parent 1:1 classid 1:200 htb rate 5mbit ceil 10mbit prio 3
tc qdisc add dev $TEST_INTERFACE parent 1:200 cake diffserv4
src/xdp_iphash_to_cpu_cmdline --add --ip 172.16.10.216 --classid 1:200 --cpu 0 --prefix 32
