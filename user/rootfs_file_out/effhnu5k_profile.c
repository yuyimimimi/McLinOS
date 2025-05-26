#include <linux/initramfs.h>

static const char filedata[] = {
    
};

register_file("/etc/profile", 0, filedata, READONLY);
