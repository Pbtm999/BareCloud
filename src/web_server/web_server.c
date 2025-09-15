void handle_request(int client_fd, struct http_request req) {
    return;
}

int start_web_server() {
    http_server_init(80, handle_request);
    return 1;
}