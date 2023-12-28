#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iomanip>

#define PORT 29000
#define WEB_SERVER_PORT 28000
#define BUFFER_SIZE 8192

// Function to get the current timestamp (https://en.cppreference.com/w/cpp/chrono/system_clock/now)
std::string current_timestamp()
{
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%F %T");
    return ss.str();
}

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    // Read the request from client
    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_read <= 0)
    {
        close(client_socket);
        return;
    }

    // Connect to tje web server
    int web_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (web_server_socket < 0)
    {
        std::cerr << "web server socket creation failed :/" << std::endl;
        close(client_socket);
        return;
    }

    struct sockaddr_in web_server_addr;
    memset(&web_server_addr, 0, sizeof(web_server_addr));
    web_server_addr.sin_family = AF_INET;
    web_server_addr.sin_port = htons(WEB_SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &web_server_addr.sin_addr); // localhost

    if (connect(web_server_socket, (struct sockaddr *)&web_server_addr, sizeof(web_server_addr)) < 0)
    {
        std::cerr << "failed to connect to web server" << std::endl;
        close(web_server_socket);
        close(client_socket);
        return;
    }

    // Forward request to web server
    send(web_server_socket, buffer, bytes_read, 0);
    std::cout << "proxy-forward,server," << std::this_thread::get_id() << "," << current_timestamp() << std::endl;

    // Receive response from web server and forward it to client
    while ((bytes_read = recv(web_server_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        send(client_socket, buffer, bytes_read, 0);
    }

    std::cout << "proxy-forward,client," << std::this_thread::get_id() << "," << current_timestamp() << std::endl;

    close(web_server_socket);
    close(client_socket);
}

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << "socket creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }

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
    if (listen(server_fd, 10) < 0)
    {
        std::cerr << "listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket < 0)
        {
            std::cerr << "accept failed" << std::endl;
            continue;
        }
        std::thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    close(server_fd);
    return 0;
}
