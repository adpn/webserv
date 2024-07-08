## WebServ
### NGINX-like Web Server

WebServ is a custom HTTP server built in C++ 98, designed to handle multiple client connections concurrently using the `poll` system call. The server listens on multiple ports and serves static HTML files in response to client requests.

Features:
  - Handles multiple client connections concurrently
  - Listens on multiple ports
  - Serves static HTML files
  - Compliant with C++ 98 for compatibility with older systems
  - Implements `GET`, `POST`, and `DELETE` HTTP methods
  - Non-blocking I/O operations
  - Provides accurate HTTP response status codes
  - Includes default error pages if none are provided
  - Allows file uploads from clients
  - Supports configuration files for customizable server settings
