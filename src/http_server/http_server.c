#include "http_server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int init_http_socket(int port) {
    int listen_socket_fd;
    struct sockaddr_in address;
    int opt = 1;

    if ((listen_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 
        || setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) 
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(listen_socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_socket_fd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    return listen_socket_fd;
}

struct http_request parse_request(char* buffer) {
    struct http_request req;
    memset(&req, 0, sizeof(req));

    char *firstLine = strtok(buffer, "\r\n");
    
    req.header_count = 0;
    char *line = strtok(NULL, "\r\n");
    req.header_count = 0;
    while (line && line[0] != '\0') {
        char *sep = strchr(line, ':');
        if (sep && req.header_count < 32) {
            *sep = '\0';
            char *name = line;
            char *value = sep + 1;

            while (*value == ' ') value++;

            strncpy(req.headers[req.header_count].name, name, sizeof(req.headers[req.header_count].name) - 1);
            strncpy(req.headers[req.header_count].value, value, sizeof(req.headers[req.header_count].value) - 1);

            req.header_count++;
        }
        line = strtok(NULL, "\r\n");
    }

    char *method = strtok(firstLine, " ");
    char *path = strtok(NULL, " ");
    char *version = strtok(NULL, " ");

    if (method)  { strncpy(req.method, method, sizeof(req.method) - 1); };
    if (path)  { strncpy(req.path, path, sizeof(req.path) - 1); };
    if (version)  { strncpy(req.version, version, sizeof(req.version) - 1); };

    return req;
}

void send_response(int client_fd, char* response) {
    write(client_fd, response, strlen(response));
}

void handle_request(int client_fd) {
    while (1) {
        char buffer[4096];
        int n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            return;
        }
        buffer[n] = '\0';
        
        printf("Recebido:\n%s\n", buffer);
        struct http_request req = parse_request(buffer);
    
        printf("Método: %s\n", req.method);
        printf("Path: %s\n", req.path);
        printf("Versão: %s\n", req.version);
    
        printf("Headers:\n");
        for (int i = 0; i < req.header_count; i++) {
            printf("  %s: %s\n", req.headers[i].name, req.headers[i].value);
        }
    
        if (strcmp(req.version, "HTTP/1.0") == 0) { keep_alive = 0; 
        else if (strcmp(req.version, "HTTP/1.1") == 0) {
            for (int i = 0; i < req.header_count; i++) {
                if (strcasecmp(req.headers[i].name, "Connection") == 0 &&
                    strcasecmp(req.headers[i].value, "close") == 0) {
                    return;
                }           
            }            
        } else {
            char body[512];
            char* status = "HTTP Version Not Supported";
            char* title = "HTTP Version Not Supported";
            char* description = "Try using HTTP/1.0 or HTTP/1.1 instead.";
    
            snprintf(body, sizeof(body),
                "<html><head><title>Pbtm PC</title></head><body><h1>505 %s</h1><p>%s</p></body></html>",
            status, description);
    
            char response[1024];
            snprintf(response, sizeof(response),
                "HTTP/1.0 505 %s\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %zu\r\n"
                "Connection: close\r\n"
                "\r\n"
                "%s",
            status, strlen(body), body);
            send_response(client_fd, response);
            return;
        }
    }
}

int http_server_init(int port, void (*callback)(int client_fd, struct http_request req)) {
    int listen_fd = init_http_socket(port);
    printf("Server listening on port %d\n", port);
    
    while (1) {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }
        
        pid_t pid = fork();

        if (pid == 0) {
            close(listen_fd);
            handle_request(client_fd);
            close(client_fd);
            exit(0);
        } else if (pid > 0) {
            close(client_fd);
        } else {
            perror("fork failed");
            close(client_fd);
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}