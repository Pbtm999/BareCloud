#include "http_server.h"
#include "web_server.h"
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

        struct redirect redirects[] = {
            { "/", "/home/" },
            { "/ticha", "/ticha/" },
            { NULL, NULL }
        };

        const char *target = NULL;
        for (int i = 0; redirects[i].from != NULL; i++) {
            if (strcmp(req.path, redirects[i].from) == 0) {
                target = redirects[i].to;
                break;
            }
        }

        int isStaticFile = 0;
        if (target) {
            char response[512];
            snprintf(response, sizeof(response),
                "HTTP/1.1 301 Moved Permanently\r\n"
                "Location: %s\r\n"
                "Content-Length: 0\r\n"
                "Connection: keep-alive\r\n"
                "\r\n", target);
            write(client_fd, response, strlen(response));
            return 0;
        } else {
            const char *clean_path = req.path[0] == '/' ? req.path + 1 : req.path;

            if (strchr(clean_path, '.') == NULL) {
                snprintf(filepath, sizeof(filepath), "static/%s/index.html", clean_path);
            } else {
                isStaticFile = 1;
                snprintf(filepath, sizeof(filepath), "static/%s", clean_path);
            }
        }

        char *buffer = NULL;
        long size = 0;
        int status = request_static_file(filepath, &buffer, &size);
        
        if (status == HTTP_NOT_FOUND) {
            printf("Not Found 404 :(\n");
            send_response(client_fd, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            return 0;
        } else if (status == HTTP_INTERNAL_ERROR) {
            printf("Error 500 :(\n");
            send_response(client_fd, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            return 0;
        }

        int client_wants_close = 0;
        for (int i = 0; i < req.header_count; i++) {
            if (strcasecmp(req.headers[i].name, "Connection") == 0) {
                if (strcasecmp(req.headers[i].value, "close") == 0) {
                    client_wants_close = 1;
                }
                break;
            }
        }
        printf("Teste here!\n");

        const char *mime = get_mime_type(filepath);

        char header[256];
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "%s"
            "\r\n",
            mime, size,
            client_wants_close || isStaticFile ? "Connection: close\r\n" : "Connection: keep-alive\r\n");
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