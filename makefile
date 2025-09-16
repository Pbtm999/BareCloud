CC = gcc
CFLAGS = -Wall -g -Isrc/web_server -Isrc/http_server

SRC = src
BUILD = build
SRCS = $(SRC)/main.c \
       $(SRC)/http_server/http_server.c \
       $(SRC)/web_server/web_server.c

OBJS = $(SRCS:$(SRC)/%.c=$(BUILD)/%.o)
TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) $(TARGET)