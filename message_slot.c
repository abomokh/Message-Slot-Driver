#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include "message_slot.h"

MODULE_LICENSE("GPL");

// Data structures
struct channel_entry {
    unsigned int channel_id;
    char data[BUFF];
    int data_size;
    struct channel_entry* next_entry;
};

struct slot_manager {
    struct channel_entry* channels_head;
};
static struct slot_manager slot_table[256];

struct file_context {
    struct channel_entry* selected_channel;
};

static struct channel_entry* find_or_create_channel(int slot_index, unsigned int channel_id);
static struct channel_entry* locate_channel(int slot_index, unsigned int channel_id);

static int open_device(struct inode* inode, struct file* file);
static long handle_ioctl(struct file* file, unsigned int command, unsigned long arg);
static ssize_t perform_read(struct file* file, char __user* user_buf, size_t size, loff_t* offset);
static ssize_t perform_write(struct file* file, const char __user* user_buf, size_t size, loff_t* offset);
static int close_device(struct inode* inode, struct file* file);

static struct file_operations device_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = handle_ioctl,
    .open           = open_device,
    .release        = close_device,
    .write          = perform_write,
    .read           = perform_read,
};

static int __init module_initialize(void) {
    int reg_result, i;
    //printk("log: inside the %s function", __FUNCTION__);
    reg_result = register_chrdev(MAJOR_NUM, DEVICE_NAME, &device_fops);
    
    if (reg_result < 0) {
        printk(KERN_ERR "Device registration failed: %d\n", MAJOR_NUM);
        return reg_result;
    }

    else {
        while (i < 256) {
            slot_table[i].channels_head = NULL;
            i++;
        }

        printk(KERN_INFO "Module loaded: %s with major number: %d\n", DEVICE_NAME, MAJOR_NUM);
        return 0;
    }
}

static int open_device(struct inode* inode, struct file* file) {
    struct file_context* context;
    
    //printk("log: Inside the %s function", __FUNCTION__);
    context = kmalloc(sizeof(struct file_context), GFP_KERNEL);
    if (!context) {
        printk("log: failuer kmalloc in msg_slot_open!!");
        return -ENOMEM;
    }
    
    else {
        printk("log: Success kmalloc in msg_slot_open!!");
        printk(KERN_INFO "Device opened with minor number: %d\n", iminor(inode));
        (*file).private_data = context;
        (*context).selected_channel = NULL;
        return 0;
    }
}

static long handle_ioctl(struct file* file, unsigned int command, unsigned long arg) {
    int slot_index;
    struct file_context* context = (struct file_context*)(*file).private_data;

    //printk("log: Inside the %s function", __FUNCTION__);
    if ((command != MSG_SLOT_CHANNEL) || (arg == 0))
        return -EINVAL;
    
    else {
        slot_index = iminor((*file).f_inode);
        (*context).selected_channel = find_or_create_channel(slot_index, arg);
        if (!(*context).selected_channel)
            return -ENOMEM;
        
        else {
            printk(KERN_INFO "IOCTL set channel %lu for device with minor %d\n", arg, slot_index);
            return 0;
        }
    }
}

static ssize_t perform_read(struct file* file, char __user* user_buf, size_t size, loff_t* offset) {
    struct file_context* context = (struct file_context*)(*file).private_data;
    struct channel_entry* active_channel = (*context).selected_channel;

    //printk("log: Inside the %s function", __FUNCTION__);
    if (!active_channel)
        return -EINVAL;

    else if (!((*active_channel).data_size))
        return -EWOULDBLOCK;

    else if ((*active_channel).data_size > size)
        return -ENOSPC;
    
    else if (copy_to_user(user_buf, (*active_channel).data, (*active_channel).data_size) != 0)
        return -EFAULT;
    
    else {
        printk(KERN_INFO "Read %d bytes from channel %d\n", (*active_channel).data_size, (*active_channel).channel_id);
        return (*active_channel).data_size;
    }
}

static ssize_t perform_write(struct file* file, const char __user* user_buf, size_t size, loff_t* offset) {
    struct file_context* context = (struct file_context*)(*file).private_data;
    struct channel_entry* active_channel = (*context).selected_channel;

    //printk("log: Inside the %s function", __FUNCTION__);
    if (!active_channel)
        return -EINVAL;

    else if (size == 0 || BUFF < size)
        return -EMSGSIZE;

    else if (copy_from_user((*active_channel).data, user_buf, size) != 0)
        return -EFAULT;
    
    else {
    (*active_channel).data_size = size;
    printk(KERN_INFO "Wrote %zu bytes to channel %d\n", size, (*active_channel).channel_id);
    return size;
    }
}

static int close_device(struct inode* inode, struct file* file) {
    //printk("log: Inside the %s function", __FUNCTION__);
    kfree((*file).private_data);
    printk(KERN_INFO "Device closed with minor number: %d\n", iminor(inode));
    return 0;
}

static struct channel_entry* find_or_create_channel(int slot_index, unsigned int channel_id) {
    struct channel_entry* chan = locate_channel(slot_index, channel_id);

    //printk("log: Inside the %s function", __FUNCTION__);
    if (!chan) {
        chan = kmalloc(sizeof(struct channel_entry), GFP_KERNEL);
        if (!chan)
            return NULL;

        else {
            (*chan).channel_id = channel_id;
            (*chan).data_size = 0;
            (*chan).next_entry = slot_table[slot_index].channels_head;
            slot_table[slot_index].channels_head = chan;
        }
    }
    return chan;
}

static struct channel_entry* locate_channel(int slot_index, unsigned int channel_id) {
    struct channel_entry* chan = slot_table[slot_index].channels_head;

    //printk("log: Inside the %s function", __FUNCTION__);
    while (chan) {
        if ((*chan).channel_id == channel_id) {
            return chan;
        }
        chan = (*chan).next_entry;
    }

    return NULL;
}

static void __exit module_cleanup(void) {
    struct channel_entry* curr;
    struct channel_entry* next;
    int i;

    //printk("log: Inside the %s function", __FUNCTION__);
    for (i = 0; i < 256; i++) {
        curr = slot_table[i].channels_head;
        while (curr) {
            next = (*curr).next_entry;
            kfree(curr);
            curr = next;
        }
    }

    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk(KERN_INFO "Module unloaded\n");
}

module_init(module_initialize);
module_exit(module_cleanup);
