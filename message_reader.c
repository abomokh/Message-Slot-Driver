#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include "message_slot.h"

#define MAX_MSG_SIZE 128  // Set maximum size for reading the message

// Function to validate command-line arguments
void check_arguments(int argc, char *argv[]) {
    //printk("log: Inside the %s function", __FUNCTION__);
    if (argc != 3) {
        fprintf(stderr, "Argument error: Usage: %s <device_path> <channel_id>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

// Function to open the message slot device
int open_message_slot(const char *device_path) {
    //printk("log: Inside the %s function", __FUNCTION__);
    int device_fd = open(device_path, O_RDWR);
    if (device_fd < 0) {
        perror("Error opening device");
        exit(EXIT_FAILURE);
    }
    return device_fd;
}

// Function to validate the channel ID
int validate_channel(const char *channel_str) {
    //printk("log: Inside the %s function", __FUNCTION__);
    int channel = atoi(channel_str);
    if (channel <= 0) {
        fprintf(stderr, "Error: Invalid channel ID: %d\n", channel);
        exit(EXIT_FAILURE);
    }
    return channel;
}

// Function to set the channel ID using ioctl
void configure_channel(int fd, int channel_id) {
    //printk("log: Inside the %s function", __FUNCTION__);
    if (ioctl(fd, MSG_SLOT_CHANNEL, channel_id) < 0) {
        perror("Error setting channel ID");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

// Function to read the message from the device
ssize_t retrieve_message(int fd, char *buffer, size_t size) {
    //printk("log: Inside the %s function", __FUNCTION__);
    ssize_t len = read(fd, buffer, size);
    if (len < 0) {
        perror("Error reading message");
        close(fd);
        exit(EXIT_FAILURE);
    }
    return len;
}

// Function to print the message
void display_message(const char *message, size_t length) {
    //printk("log: Inside the %s function", __FUNCTION__);
    if (write(STDOUT_FILENO, message, length) < 0) {
        perror("Error writing message to stdout");
        exit(EXIT_FAILURE);
    }
}

// Main function
int main(int argc, char *argv[]) {
    //printk("log: Inside the %s function", __FUNCTION__);
    check_arguments(argc, argv);

    int device_fd = open_message_slot(argv[1]);
    int channel = validate_channel(argv[2]);

    configure_channel(device_fd, channel);

    char message[MAX_MSG_SIZE];
    ssize_t message_length = retrieve_message(device_fd, message, sizeof(message));

    display_message(message, message_length);

    close(device_fd);
    return 0;
}
