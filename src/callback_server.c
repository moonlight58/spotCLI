#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Returns the authorization code, or NULL on failure
char* start_callback_server(int port, char *code_buffer, size_t buffer_size) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[4096] = {0};
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return NULL;
    }
    
    // Allow reuse of address
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return NULL;
    }
    
    if (listen(server_fd, 1) < 0) {
        perror("listen failed");
        close(server_fd);
        return NULL;
    }
    
    printf("✓ Callback server listening on http://127.0.0.1:%d\n", port);
    printf("Waiting for authorization...\n\n");
    
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept failed");
        close(server_fd);
        return NULL;
    }
    
    // Read the HTTP request
    read(client_fd, buffer, sizeof(buffer) - 1);
    
    // Extract code from GET request
    char *code_start = strstr(buffer, "code=");
    if (!code_start) {
        const char *response = 
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<html><body><h1>Error: No authorization code received</h1></body></html>";
        write(client_fd, response, strlen(response));
        close(client_fd);
        close(server_fd);
        return NULL;
    }
    
    code_start += 5; // Skip "code="
    char *code_end = strstr(code_start, " HTTP");
    if (!code_end) code_end = strchr(code_start, '&');
    if (!code_end) code_end = strchr(code_start, '\r');
    if (!code_end) code_end = strchr(code_start, '\n');
    
    size_t code_len = code_end ? (size_t)(code_end - code_start) : strlen(code_start);
    if (code_len >= buffer_size) code_len = buffer_size - 1;
    
    strncpy(code_buffer, code_start, code_len);
    code_buffer[code_len] = '\0';
    
    // Send success response
    const char *response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html><body style='font-family: Arial; text-align: center; padding: 50px; background-color: #141414;'>"
        "<h1 style='color: #1DB954;'>Authorization Successful!</h1>"
        "<p style='color: #1DB954;'>You can close this window and return to the terminal.</p>"
        "</body></html>";
    
    write(client_fd, response, strlen(response));
    
    close(client_fd);
    close(server_fd);
    
    return code_buffer;
}
