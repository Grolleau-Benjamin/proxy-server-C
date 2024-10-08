# Proxy Server Project in C

## Introduction

This project is a simple HTTP proxy server written in C. It intercepts HTTP requests from clients and forwards them to the destination servers. The proxy can filter content based on configurable rules, log requests, and manage configurations dynamically.

## Features

- **HTTP Proxy Server**: Handles HTTP requests and responses between clients and servers.
- **Content Filtering**: Blocks access to specified domains or content based on keywords and configurable rules.
- **Logging**: Records proxy activities, requests, and errors in a log file.
- **Dynamic Configuration**: Loads configuration parameters and filtering rules from external files.

## Directory Structure

```
.
├── includes                 // Header files for the modules
│   ├── config.h
│   ├── http_helper.h
│   ├── logger.h
│   ├── rules.h
│   ├── server.h
│   └── utils.h
├── main.c                   // The main entry point of the proxy 
├── Makefile                 // Instructions for building the project
├── obj/                     // Directory where object files are stored after compilation
├── conf/                    // Directory where config file are stored
│   ├── proxy.config         // Configuration file for the proxy server
│   └── proxy.rules          // Rules file defining filtering criteria
├── logs/
│   └──proxy.log             // Log file where activities are recorded
├── README.md
├── src                      // Source files implementing the functionality
│   ├── config.c
│   ├── http_helper.c
│   ├── logger.c
│   ├── rules.c
│   ├── server.c
│   └── utils.c
├── Doxyfile                 // Configuration file for doxygen
├── Doxygen.md               // Main page for doxygen doc generation
├── docs/html/index.html     // Entry point of the documentation
└── test/                    // Directory where test source files are
    ├── test_http_helper.c
    └── test_server.c
```

## Installation

### Prerequisites

- GCC Compiler
- Make utility
- Standard C libraries
- **Doxygen** (for documentation generation)

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
  
  Two ways are available to build the project:
  
  - **Standard Build**
    
    Run the basic `make` command to compile the source code:
    
    ```bash
    make
    ```
    
  - **Debug Build**
    
    Run the `make debug` command to compile the source code in debug mode:
    
    ```bash
    make debug
    ```
    
  
  This compiles the source files and generates an executable `proxy` or `proxy_debug`, depending on the command.
  
4. **Run Tests (Optional)**
  
  Run `make test` to execute all tests and verify if the program is working as expected.
  
  ```bash
  make test
  ```
  
5. **Generate Documentation**
  
  Run the `make docs` command to generate the project's documentation using Doxygen:
  
  ```bash
  make docs
  ```
  
  The documentation will be generated in the `docs/` directory. You can open the `docs/html/index.html` file in your web browser to view the documentation.
  
6. **Clean Build (Optional)**
  
  To clean up object files and executables:
  
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
- **ADDRESS**: The IPv4 address on which the proxy server is bound.
- **MAX_CLIENT**: The maximum number of simultaneous client connections.
- **LOGGER_FILENAME**: The file where logs are recorded.
- **RULES_FILENAME**: The file containing filtering rules.

**Modifying Configuration**

Edit the `proxy.config` file to change the proxy server's settings. The server reads this file at startup, so don't forget to restart it to apply changes.

### proxy.rules

The `proxy.rules` file defines the filtering rules used by the proxy to block or allow content.

**Rules Format**

The rules are defined in a simple format:

```bash
[Category]
# List of banned domains
BAN_DOMAIN example.com
BAN_DOMAIN badsite.com
# List of banned words
BAN_WORD forbidden
BAN_WORD secret
```

**Rules Types**

- **BAN_DOMAIN**: Blocks access to the specified domain.
- **BAN_WORD**: Blocks content containing the specified keyword.

**Applying Rules**

The proxy server loads rules at startup and applies these rules to each HTTP request it processes.

## Usage

1. **Start the Proxy Server**
  
  Run `./proxy` or `./proxy_debug` after building it. The server will start listening on the configured address and port.
  
2. **Configure Clients to Use the Proxy**
  
  Set up your web browser or HTTP client to use the proxy server. For example, configure the proxy settings to `127.0.0.1:8080`.
  
3. **Access HTTP Websites**
  
  Use the client to browse websites as usual. The proxy will handle the requests according to the defined rules.
  

## Logging

- **Log File**: The proxy activities are recorded in the file specified by `LOGGER_FILENAME` in `proxy.config` (default is `proxy.log`).
- **Log Levels**: The proxy records information, warnings, and errors.

## Generating Documentation

The project's documentation is generated using Doxygen, which helps in understanding the code and modules. To generate the documentation, ensure that Doxygen is installed, then run:

```bash
make docs
```

The documentation will be generated in the `docs/` directory. Open `docs/html/index.html` in your web browser to view it.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
