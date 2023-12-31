# Isolation Tester

## Overview
Isolation Tester is a diagnostic tool designed to run inside a sandbox or other isolation environments. Its primary purpose is to evaluate and report on the information that the application can access while being confined within these controlled environments. This tool is particularly useful for testing the effectiveness of isolation mechanisms in restricting access to system resources and network interfaces.

## Features
The Isolation Tester performs a series of checks and outputs relevant information including:
- **Capabilities**: Lists the capabilities of the process within the sandbox.
- **User/PID Info**: Displays the User ID, Group ID, and Process ID.
- **Directories**: Lists directories, specifically aiming to enumerate the root directory.
- **Network Interfaces**: Enumerates network interfaces and their IP addresses.
- **Running Processes**: Lists processes currently running on the system, focusing on those in `/proc`.

## Building and Running
### Prerequisites
- A C++ compiler (preferably `g++`).
- The `libcap` library for handling capabilities.

### Compilation
Use the provided Makefile to compile the program. The Makefile contains targets for building, cleaning, and running the application.

To compile, use:
```
make
```

### Running
To run the program after compilation, use:
```
make run
```

Alternatively, you can run the executable directly using:
```
./isolation_tester
```

## Usage
This tool is intended to be run in a sandboxed or otherwise isolated environment. After launching the application in such an environment, it will output data about the system and its resources as seen from within the isolation. This output can then be used to assess the effectiveness of the isolation mechanism.

## Caution
Ensure that you understand the implications of running diagnostic tools like Isolation Tester, especially in sensitive or production environments. The tool should be used responsibly and primarily in test environments for evaluation purposes.

<!-- ## Contribution
Feedback and contributions to the Isolation Tester are welcome. Please ensure that any contributions adhere to best practices for security and efficiency.

## License
[Specify License Here] -->
