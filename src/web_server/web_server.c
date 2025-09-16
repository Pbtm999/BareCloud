#include "http_server.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define HTTP_OK 0
#define HTTP_NOT_FOUND 404
#define HTTP_INTERNAL_ERROR 500

int request_static_file(char* path, char **out_buffer, long *out_size) {
    FILE *file = fopen(path, "rb");
    if (!file)
        return 404;
    
    fseek(file, 0, SEEK_END);

    long file_size = ftell(file);
    rewind(file);

    char *buffer = malloc(file_size + 1);
    size_t read_bytes = fread(buffer, 1, file_size, file);
    if (read_bytes != file_size) {
        free(buffer);
        fclose(file);
        return 500;
    }
    buffer[file_size] = '\0';
    fclose(file);

    *out_buffer = buffer;
    *out_size = file_size;
    return 0;
}

const char* get_mime_type(const char *path) {
    if (strstr(path, ".html")) return "text/html";
    if (strstr(path, ".css"))  return "text/css";
    if (strstr(path, ".js"))   return "application/javascript";
    if (strstr(path, ".ico"))  return "image/x-icon";
    if (strstr(path, ".svg"))  return "image/svg+xml";
    if (strstr(path, ".png"))  return "image/png";
    if (strstr(path, ".jpg") || strstr(path, ".jpeg")) return "image/jpeg";
    return "application/octet-stream";
}

int handle_request(int client_fd, struct http_request req) {
    printf("Request: %s %s\n", req.method, req.path);

    if (strcmp(req.method, "GET") == 0) {
        char filepath[512];

        if (strcmp(req.path, "/") == 0) {
            snprintf(filepath, sizeof(filepath), "static/build/index.html");
        } else {
            snprintf(filepath, sizeof(filepath), "static/build%s", req.path);
        }

        char *buffer = NULL;
        long size = 0;
        int status = request_static_file(filepath, &buffer, &size);
        
        if (status == HTTP_NOT_FOUND) {
            send_response(client_fd, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
            return 0;
        } else if (status == HTTP_INTERNAL_ERROR) {
            send_response(client_fd, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
            return 0;
        }

        const char *mime = get_mime_type(filepath);

        char header[256];
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "Connection: close\r\n"
            "\r\n", mime, size);

        write(client_fd, header, strlen(header));
        write(client_fd, buffer, size);

        free(buffer);
        return 0;
        
    } else if (strcmp(req.method, "POST") == 0) {
        send_response(client_fd, "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n");
        return 0;
    } else {
        send_response(client_fd, "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n");
        return 0;
    }

    return 1;
}

int start_web_server(int port) {
    http_server_init(port, handle_request);
    return 1;
}