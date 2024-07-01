# Multi-Client Network Server

A simple TCP server implemented in C that handles basic GET and PUT commands from clients. The server can handle multiple client connections using the `fork()` system call for concurrent processing.

## Features

- **Process Management**: Utilizes `fork()` to handle multiple clients concurrently.
- **Socket Programming**: Demonstrates basic TCP socket operations.
- **GET and PUT Operations**: Supports GET to read and send file contents to the client, and PUT to receive and write data from the client to a file.
- **Error Handling**: Provides appropriate error messages for various failure scenarios.

## Technologies Used

- **Language**: C
- **Libraries**: Standard C libraries, Socket programming
- **System Calls**: `fork()`, `socket()`, `bind()`, `listen()`, `accept()`, `recv()`, `send()`, `fopen()`, `fgetc()`, `fwrite()`

## Usage

1. Clone the repository.
2. Compile the `tcp_server.c` file using a C compiler.

#### Compiling the C file

```bash
gcc -o tcp_server tcp_server.c
```

#### Running the executable

3. Run the compiled executable providing the port number as a command-line argument.

```bash
./tcp_server <port_number>
```
Replace <port_number> with the desired port number (must be greater than 1024).

## Acknowledgements
These implementations were created for a NWEN241 assignment at Victoria University of Wellington.
