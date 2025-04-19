#!/bin/sh
cd test
python3 ./simulate_serial.py &

cd ..
sleep 3

while [ true ]
do
    ./bin/telemetrycore -c ./test.cfg -s /dev/pts/1 -e 10000 --http-port 9000 --ws-port 9001 -a ./client-roadrunner
done