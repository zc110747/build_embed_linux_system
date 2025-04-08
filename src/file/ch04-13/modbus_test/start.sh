
pkill -9 target
pkill -9 modbus_test

../modbus_s/target &
sleep 2
./modbus_test &
