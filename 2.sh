#!/bin/sh
# https://stackoverflow.com/questions/596590/how-can-i-get-the-current-network-interface-throughput-statistics-on-linux-unix/674400#674400
S=1; F=/sys/class/net/eth0/statistics/tx_bytes
TXS=999999
while [ $TXS -gt 1 ]
do
  X=`cat $F`; sleep $S; Y=`cat $F`; TXS="$(((Y-X)/S))";
  echo TXS is currently $TXS
done
echo 'ALARM TXS is low'

