# Message Slot Driver

## Overview
This project implements a character device driver for Linux named `message_slot`. The driver allows processes to communicate by writing and reading messages via specific channels within a device file.

## Components
The project includes the following files:

1. **`message_slot.h`**: Contains constants, macros, and function declarations used by the driver and user-space programs.
2. **`message_slot.c`**: Implements the device driver, supporting channel-based message reading and writing.
3. **`message_sender.c`**: A user-space program for writing messages to a specific channel in a device file.
4. **`message_reader.c`**: A user-space program for reading messages from a specific channel in a device file.
5. **`Makefile`**: Automates the build process for compiling the kernel module and user-space programs.

## Features
- Support for multiple channels per device file.
- Message storage up to 128 bytes per channel.
- Standard Linux file operations: `open`, `read`, `write`, `ioctl`, and `release`.
- Communication between processes using the same channel.

## Usage

### Compilation
Use the provided `Makefile` to compile the kernel module and user-space programs:
```bash
make
```
This will produce the following outputs:
- `message_slot.ko`: The kernel module.
- `message_sender`: The user-space sender program.
- `message_reader`: The user-space reader program.

### Loading the Module
Load the `message_slot` module into the kernel:
```bash
sudo insmod message_slot.ko
```

### Creating a Device File
Create a character device file:
```bash
sudo mknod /dev/message_slot c 235 0
```

### Running the User Programs
#### Writing a Message
Use the `message_sender` program to write a message:
```bash
./message_sender /dev/message_slot <channel_id> "<message>"
```
Example:
```bash
./message_sender /dev/message_slot 1 "Hello, World!"
```

#### Reading a Message
Use the `message_reader` program to read a message:
```bash
./message_reader /dev/message_slot <channel_id>
```
Example:
```bash
./message_reader /dev/message_slot 1
```

### Unloading the Module
After use, unload the kernel module:
```bash
sudo rmmod message_slot
```

### Cleaning Up
Remove compiled files:
```bash
make clean
```

## Implementation Details

### Driver (`message_slot.c`)
- **Channels**: Implemented as linked lists where each node corresponds to a channel.
- **File Context**: Stores the currently selected channel for each file descriptor.
- **IOCTL**: Used to set the active channel.
- **Read/Write**:
  - `read`: Retrieves the message stored in the selected channel.
  - `write`: Stores a message in the selected channel, up to 128 bytes.

### User Programs
- **`message_sender.c`**: Opens the device file, sets the channel via `ioctl`, and writes a message.
- **`message_reader.c`**: Opens the device file, sets the channel via `ioctl`, and reads a message to display on stdout.

## Error Handling
The driver and programs handle errors such as:
- Invalid channel ID.
- Message size exceeding the buffer limit.
- Channel or device not set.
- Copying data between user and kernel space.

## Requirements
- Linux kernel headers.
- GCC for compilation.

## License
This project is licensed under the GPLv2.

---

For questions or contributions, feel free to contact the maintainer.

