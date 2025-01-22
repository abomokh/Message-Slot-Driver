#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include "message_slot.h"

void validate_arguments(int argc, char *argv[]);
int open_message_slot(const char *file_path);
void set_channel(int fd, int channel);
void write_message(int fd, const char *message);

int main(int argc, char *argv[]) {
    //printk("log: Inside the %s function", __FUNCTION__);
    int message_slot_fd;
    int channel;

    validate_arguments(argc, argv);
    message_slot_fd = open_message_slot(argv[1]);
    channel = atoi(argv[2]);
    set_channel(message_slot_fd, channel);
    write_message(message_slot_fd, argv[3]);

    close(message_slot_fd);
    return 0;
}

void validate_arguments(int argc, char *argv[]) {
    //printk("log: Inside the %s function", __FUNCTION__);
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file_path> <channel_id> <message>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

int open_message_slot(const char *file_path) {
    //printk("log: Inside the %s function", __FUNCTION__);
    int fd = open(file_path, O_RDWR);

    if (fd < 0) {
        perror("Failed to open the device file");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void set_channel(int fd, int channel) {
    //printk("log: Inside the %s function", __FUNCTION__);
    if (channel <= 0) {
        fprintf(stderr, "Invalid channel id: %d\n", channel);
        exit(EXIT_FAILURE);
    }
    if (ioctl(fd, MSG_SLOT_CHANNEL, channel) < 0) {
        perror("Failed to set channel id");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

void write_message(int fd, const char *message) {
    //printk("log: Inside the %s function", __FUNCTION__);
    ssize_t message_length = write(fd, message, (int)strlen(message));

    if (message_length < 0) {
        perror("Failed to write message");
        close(fd);
        exit(EXIT_FAILURE);
    }
}