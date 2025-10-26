#ifndef WEB_SERVER_H
#define WEB_SERVER_H

int start_web_server(int port);

struct redirect {
    const char *from;
    const char *to;
};

#endif