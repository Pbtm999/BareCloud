#ifndef WEB_SERVER_H
#define WEB_SERVER_H

void start_web_server(int port, void (*callback)(int client_fd, struct http_request req));

#endif