### SUMMARY

- This project implements a multithreaded Proxy Web Server in C++, incorporating concurrent request handling over the TCP/IP stack.
- The system is designed to seamlessly integrate a proxy server, optimizing the flow of requests to the server and responses back to the client. 

### INSTRUCTIONS

- Run the make command from the top-level directory to build the program.
- Launch the web server by executing `"./web/webserver"` and the proxy server by executing `"./proxy/proxy server"`.
- Open your web browser and visit `"localhost:29000"` (proxy port) or `"localhost:28000"` (web port)
- Access any file by visiting `"localhost:<port>/<relative file path from root>"`
