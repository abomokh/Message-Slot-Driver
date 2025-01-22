#ifndef MSG_SLOT_H
#define MSG_SLOT_H
#include <linux/ioctl.h>

#define DEVICE_NAME "message_slot"
#define BUFF 128
#define MAJOR_NUM 235
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int)
#endif