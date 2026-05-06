# msg_framework

A lightweight inter-module message communication framework for embedded systems, supporting RT-Thread, FreeRTOS and Zephyr.

## Features

- **Cross-platform**: Supports RT-Thread, FreeRTOS, Zephyr
- **Lightweight**: ~300 lines of core code, zero dependencies
- **Async communication**: Message queue based inter-module communication
- **Easy to use**: Simple API design

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         msg_framework                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐  │
│   │ Module A │    │ Module B │    │ Module C │    │ Module D │  │
│   └────┬─────┘    └────┬─────┘    └────┬─────┘    └────┬─────┘  │
│        │               │               │               │         │
│        │  mf_send_msg  │               │               │         │
│        │   ───────────► │               │               │         │
│        │               │               │               │         │
│        │    ┌──────────────────────────────┐          │         │
│        │    │       Message Queue Hub       │          │         │
│        │    │  ┌─────────────────────────┐ │          │         │
│        │    │  │  MSG Queue for Module B │ │          │         │
│        │    │  │  [msg1] [msg2] [msg3]   │ │          │         │
│        │    │  └─────────────────────────┘ │          │         │
│        │    └──────────────────────────────┘          │         │
│        │               │               │               │         │
│        │               │  mf_recv_msg  │               │         │
│        │               │  ◄─────────── │               │         │
│        │               │               │               │         │
└────────┴───────────────┴───────────────┴───────────────┴─────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │   OS Port Layer  │
                    ├─────────────────┤
                    │ rtt/            │
                    │ freertos/       │
                    │ zephyr/         │
                    └─────────────────┘
```

## Advantages

### 1. Low Coupling
- Modules communicate via messages, no direct references needed
- Modules can be compiled, tested and reused independently
- Adding new modules doesn't affect existing ones

### 2. Async Communication
- Returns immediately after sending, no need to wait for processing
- Receiving module processes messages at its own pace
- Improves system responsiveness and throughput

### 3. Traceability
- Messages carry source/destination module IDs
- Message codes are managed centrally, behavior is predictable
- Easy for debugging and log analysis

### 4. Easy Integration
- Just implement `mf_port.h` interface to support new OS
- Core code is OS-agnostic
- Suitable for resource-constrained embedded systems

### 5. Memory Safety
- Framework automatically manages message data copying
- Caller doesn't need to manage data lifecycle
- Avoids memory leaks and wild pointers

## Quick Start

### Initialize Framework

```c
mf_config_t config = {
    .enable_debug = 1,
};
mf_init(&config);
```

### Bind Module

```c
// Call once during module initialization
mf_module_bind(MODULE_ID, "module_a", sizeof(AppMsg), 16);
```

### Send Message

```c
AppMsg msg = {...};
mf_send_msg(SRC_MODULE, DEST_MODULE, MSG_ID, 0, &msg, sizeof(msg));
```

### Receive Message

```c
mf_message_t msg;
int ret = mf_recv_msg(MY_MODULE_ID, &msg, sizeof(msg), 1000);
if (ret == MF_OK) {
    // Process message
    AppMsg* p = (AppMsg*)msg.pdata;
    mf_free_msg(&msg);  // Must call to release
}
```

## Build

### Using CMake

```bash
# RT-Thread version
mkdir build && cd build
cmake ..
make

# FreeRTOS version
cmake -DUSE_FREERTOS=ON ..
make

# Zephyr version
cmake -DUSE_ZEPHYR=ON ..
make
```

### Embed into Existing Project

Add `include/` and `src/` directories to your project, then select the appropriate port file for your target OS.

## API Reference

| API | Description |
|-----|-------------|
| `mf_init()` | Initialize framework |
| `mf_version()` | Get version string |
| `mf_module_bind()` | Bind module to message queue |
| `mf_module_unbind()` | Unbind module |
| `mf_module_is_bound()` | Check if module is bound |
| `mf_send_msg()` | Send message |
| `mf_recv_msg()` | Receive message |
| `mf_free_msg()` | Free message memory |
| `mf_flush()` | Flush module queue |
| `mf_set_debug_cb()` | Set debug callback |

## Message Structure

```c
typedef struct {
    uint8_t  src_moduleid;   // Source module ID
    uint8_t  dest_moduleid;  // Destination module ID
    uint32_t msg_code;       // Message code (defined by application)
    uint32_t msg_para;       // Message parameter
    void*    pdata;          // Data pointer
    uint16_t datalen;        // Data length
} mf_message_t;
```

## Directory Structure

```
msg_framework/
├── include/                 # Public header
│   └── msg_framework.h
├── src/                     # Core implementation
│   └── msg_framework.c
├── port/                    # OS adaptation layer
│   ├── mf_port.h           # Port interface
│   ├── rtt/                # RT-Thread port
│   │   └── mf_port_rtt.c
│   ├── freertos/           # FreeRTOS port
│   │   └── mf_port_freertos.c
│   └── zephyr/             # Zephyr port
│       └── mf_port_zephyr.c
├── tests/                   # Unit tests
└── CMakeLists.txt
```

## Adding New OS Port

To support other RTOS:

1. Create a new directory under `port/`, e.g., `myos/`
2. Implement the interfaces defined in `mf_port.h`
3. Add corresponding option and source file in CMakeLists.txt

## Error Codes

| Code | Meaning |
|------|---------|
| MF_OK | Success |
| MF_ERR | General error |
| MF_EEMPTY | Resource not bound |
| MF_ENOMEM | Memory allocation failed |
| MF_EFULL | Queue full |
| MF_EINVAL | Invalid parameter |
| MF_ETIMEOUT | Timeout |
