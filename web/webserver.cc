#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#define PORT 28000

// std::vector<pthread_t> threads; // Holds all threads
bool server_flag = true;

std::string handle_request(const std::string &request)
{
    // Parse first line of request.
    std::istringstream iss(request);
    std::string method, path, protocol;
    iss >> method >> path >> protocol;
    // Return Not Allowed if not a GET request.
    if (method != "GET")
    {
        return protocol + " 405 Method Not Allowed";
    }
    // Return 404 if file not found.
    std::ifstream requested_file("." + path, std::ios::binary);
    if (!requested_file.is_open())
    {
        return protocol + " 404 Not Found";
    }
    // Determine content type of file.
    std::string content_type;
    if (path.substr(path.find_last_of('.') + 1) == "html")
    {
        content_type = "text/html"; // Webpage.
    }
    else if (path.substr(path.find_last_of('.') + 1) == "pdf")
    {
        content_type = "application/pdf"; // PDF File.
    }
    else
    {
        content_type = "text/plain"; // Plain Text.
    }
    // Copy file into buffer.
    std::stringstream raw_file_stream;
    raw_file_stream << requested_file.rdbuf();
    std::string raw_file_string = raw_file_stream.str();
    // Generate Response.
    std::ostringstream response;
    response << protocol << " 200 OK\r\n";
    response << "Content-Length: " << raw_file_string.length() << "\r\n";
    response << "Content-Type: " << content_type << "\r\n\r\n";
    response << raw_file_string;
    return response.str();
}

void *handle_client(void *client_socket)
{
    int client_fd = *(int *)client_socket;
    delete (int *)client_socket;
    // Same logic as webserver1.
    char buffer[1024] = {0};
    read(client_fd, buffer, 1024);
    std::string response_str = handle_request(buffer);
    for (size_t i = 0; i < response_str.size(); i += 1024)
    {
        size_t chunk_size = std::min(response_str.size() - i, static_cast<size_t>(1024));
        send(client_fd, response_str.c_str() + i, chunk_size, 0);
    }
    // std::cout << "CLIENT_FD: " << client_fd << std::endl;
    // std::cout << response_str << std::endl;

    close(client_fd);
    return nullptr;
}

void stop_server(int signum)
{
    // Stops server when ctrl+c is pressed.
    server_flag = false;
}

int main(int argc, char const *argv[])
{
    signal(SIGINT, stop_server);

    // Create Socket that communicates over the internet and uses TCP/IP.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "socket creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    // Set socket options for server_fd to reuse address and port at socket level.
    int option = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        std::cerr << "setsockopt SO_REUSEADDR failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option)))
    {
        std::cerr << "setsockopt SO_REUSEPORT failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Create an ipv4 sockaddr (socket address).
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    // Bind socket to address and port in sockaddr.
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    // Listen for clients.
    if (listen(server_fd, 3) < 0)
    {
        std::cerr << "listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    // Print success message.
    std::cout << "Server started successfully on port " << PORT << std::endl;

    // Handle client connections.
    int client_fd;
    int address_len = sizeof(address);
    fd_set set;
    struct timeval timeout;

    while (server_flag)
    {
        FD_ZERO(&set);
        FD_SET(server_fd, &set);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int rv = select(server_fd + 1, &set, NULL, NULL, &timeout);
        if (rv == 0 || rv == -1)
        {
            continue;
        }

        // Accept client connection.
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&address_len);
        if (client_fd < 0)
        {
            if (!server_flag)
            {
                std::cout << "Shutting down server" << std::endl;
                break;
            }
            std::cerr << "accept failed" << std::endl;
            continue;
        }
        pthread_t client_thread;
        int *client_socket = new int(client_fd);
        if (pthread_create(&client_thread, nullptr, handle_client, client_socket))
        {
            std::cerr << "thread creation failed" << std::endl;
            exit(EXIT_FAILURE);
        }
        pthread_detach(client_thread);
    }

    std::cout << "\nShutdown successful" << std::endl;
    close(server_fd);

    return 0;
}
