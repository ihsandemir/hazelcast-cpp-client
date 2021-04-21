## Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

> You may need to change or comment `set(CMAKE_PREFIX_PATH "/usr/local/hazelcast")` in `CMakeLists.txt`.

## Run

### Reliable topic: deadlock

`./publisher rtopic` => publish a string in a reliable topic (in a loop)

`./subscriber V1` => subscribe to a reliable topic (deadlock)

### Reliable topic: no deadlock

`./publisher rtopic` => publish a string in a reliable topic (in a loop)

`./subscriber V2`

### Topic: no deadlock

`./publisher topic` => publish a string in a topic (in a loop)

`./subscriber V3`
