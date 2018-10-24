#include <linux/init.h>
#include <linux/module.h>
#include <linux/virtio.h>
#include <linux/virtio_ids.h>

#ifndef VIRTIO_ID_MINI
#define VIRTIO_ID_MINI 21
#endif

MODULE_AUTHOR("Matthias Prangl");
MODULE_DESCRIPTION("virtio example front-end driver");
MODULE_LICENSE("GPL v2");

static struct virtio_device_id id_table[] = {
	{ VIRTIO_ID_MINI, VIRTIO_DEV_ANY_ID },
	{ 0 },
};

static unsigned int feature_table[] = { };

int probe_virtio_mini(struct virtio_device *dev) {
    printk(KERN_INFO "virtio-mini device found\n");
    return 0;
}

void remove_virtio_mini (struct virtio_device *dev) {
    printk(KERN_INFO "virtio-mini device removed\n");
}

static struct virtio_driver driver_virtio_mini = {
    .driver.name = KBUILD_MODNAME,
    .driver.owner = THIS_MODULE,
    .id_table = id_table,
    .feature_table = feature_table,
    .feature_table_size = ARRAY_SIZE(feature_table),
    .probe = probe_virtio_mini,
    .remove = remove_virtio_mini
};

static int __init init_virtio_mini(void) {
    int error = register_virtio_driver(&driver_virtio_mini);
    if(error) {
        printk(KERN_INFO "virtio_mini Error: error %i", error);
        return -1;
    }
    printk(KERN_INFO "virtio_mini module initialized\n");
    return 0;
}

static void __exit exit_virtio_mini(void) {
    printk(KERN_INFO "virtio_mini module removed\n");
}

module_init(init_virtio_mini);
module_exit(exit_virtio_mini);
