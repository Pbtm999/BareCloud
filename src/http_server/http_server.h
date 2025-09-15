#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

struct http_header {
    char name[64];
    char value[256];
};

struct http_request {
    char method[16];
    char path[256];
    char version[16];
    struct http_header headers[32];
    int header_count;
};

int http_server_init(int port, void (*callback)(int client_fd, struct http_request req));
void send_response(int client_fd, char *response)

#endif