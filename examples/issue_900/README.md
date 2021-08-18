## Build

```bash
mkdir build
cd build
cmake .. -DWITH_EXAMPLES=ON
make -j q_subscriber
make -j q_publisher
```

## Run

1. start server
2. Run q_publisher
3. Run q_subscriber
4. Send SIGTERM to q_subscriber