## How to use
1. Build the test tool:
```bash
cd e810_dtu/
make
```

2. config network (direct conn)
```bash
# set static ip on miniPC
sudo ./shell/config_network.sh
```

3. run full tests
```bash
# full test
./shell/connection.sh


# or run individual tests
./bin/test-e810 --ping
./bin/test-e810 --tcp
./bin/test-e810 --modbus

# all tests
./bin/test-e810 -i 192.168.4.101 -p 8886
```




