# Proxy Server Projet in C

## Introduction

This projet is a simple HTTP proxy server written in C. It intercepts HTTP requests from clients and forwards them to the destination servers. The proxy can filter content based on configurables rules, log requests, and manage configurations dynamically.

## Features

- **HTTP Proxy Server**: Handles HTTP requests and responses between clients and servers.
- **Content Filtering**: Block access to specified domains or content based with keywords on configurable rules.
- **Logging**: Records proxy activities, requests, and errors in a log file.
- **Dynamic Configuration**: Loads configuration parameters and filtering rules from external files.

## Directory Structure

```
.
├── includes                // Header files for the modules
│   ├── config.h
│   ├── http_helper.h
│   ├── logger.h
│   ├── rules.h
│   ├── server.h
│   └── utils.h
├── main.c                  // The main entry point of the proxy 
├── Makefile                // Instructions for building the project
├── obj/                    // Directory where object files are stored after compilation
├── proxy.config            // Configuration file for the proxy server
├── proxy.log               // Log file where activities are recorded
├── proxy.rules             // Rules file defining filtering criteria
├── README.md
└── src                     // Source files implementing the functionality
    ├── config.c
    ├── http_helper.c
    ├── logger.c
    ├── rules.c
    ├── server.c
    └── utils.c
```

## Installation

### Prerequisites

- GCC Compiler

- Make utility

- Standard C libraries

### Building the Project

1. **Clone the Repository**

```bash
git clone https://github.com/Grolleau-Benjamin/proxy-server-C.git
```

2. **Navigate to the Project Directory**

```bash
cd proxy-server-C
```

3. **Build the Project**

Two way are available to build the project:

Run the basic `make` command to compile the source code: 

```bash
make
```

Run the `make debug` command to compile the source code in debug mode: 

```bash
make debug
```

This compile the source files and generate an executable `proxy`or `proxy_debug` , depending on the command.

Finally, run `make test` to run all test and look if the programme is working as expected.
```bash
make test
```

4. **Clean build (Optionnal)**

To clean up objects and executables: 

```bash
make clean
```

## Configuration

### proxy.config

The `proxy.config` file contains configuration parameters for the proxy server. The default parameters are: 

```bash
# Configuration file for the proxy server

PORT 8080
ADDRESS 127.0.0.1
MAX_CLIENT 10
LOGGER_FILENAME proxy.log
RULES_FILENAME proxy.rules
```

**Parameters**

- **PORT**: The port number on which the proxy server listens.

- **ADDRESS**: The address IPv4 on which the proxy server is bind to.

- **MAX_CLIENT**: The maximum number of simultaneous clients connections.

- **LOGGER_FILENAME**: The file where logs are recoreded.

- **RULES_FILENAME**: The file containing filtering rules.

**Modifying Configuration**

Edit the `proxy.config` file to change the proxy server's settings. The server reads this file at startup so don't forget to restart it to have changes. 

### proxy.rules

The `proxy.rules` file defines the filtering rules used by the proxy to block or allow content. 

**Rules Format**

The rules are defined in a simple format: 

```bash
[Categorie]
# List of banned domains
BAN_DOMAIN example.com
BAN_DOMAIN badsite.com
# List of banned words
BAN_WORD forbidden
BAN_WORD secret
```

**Rules Types**

- **BAN_DOMAIN**: Blocks access to the specified domain.

- **BAN WORD**: Blocks content containing the specified keyword.

**Applying Rules**

The proxy server loads rules at startup and applies these rules to each HTTP request it processes.

### Usage

1. **Start the Proxy Server**

Run `./proxy` or `./proxy_debug` after building it. The server will start listening on the configured address and port.

2. **Configure Clients to Use the Proxy**

Set up your web browser or HTTP client to use the proxy server. For example, configure the proxy settings to `127.0.0.1:8080`.

3. **Access HTTP Website**

Use the client to browse websites as usual. The proxy will handle the requests according to the defined rules.

### Logging

- **Log File**: The proxy activities are recorded in the file specified by `LOGGER_FILENAME` in `proxy.config` (default is `proxy.log`).

- **Log Levels**: The proxy records information, warnings, and errors.

### License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
