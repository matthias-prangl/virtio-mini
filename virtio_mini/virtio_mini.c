#include <linux/module.h>
#include <linux/virtio.h>
#include <linux/virtio_ids.h>
#include <linux/virtio_config.h>

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

struct virtio_mini_info {
    struct virtqueue *vq;
};

void virtio_mini_vq_callback(struct virtqueue *vq) {
    printk(KERN_INFO "callback !\n");
}

int probe_virtio_mini(struct virtio_device *dev) {
    struct virtio_mini_info *vmini;
    
    printk(KERN_INFO "virtio-mini device found\n");
    vmini = kzalloc(sizeof(struct virtio_mini_info), GFP_KERNEL);
    if(vmini == NULL) {
        return ENOMEM;
    }
    vmini->vq = virtio_find_single_vq(dev, virtio_mini_vq_callback, "mini-queue");
    if(vmini->vq) {
        printk(KERN_INFO "virtualqueue assigned: %s\n", vmini->vq->name);
    }
    dev->priv = vmini;
    return 0;
}

void remove_virtio_mini (struct virtio_device *dev) {
    printk(KERN_INFO "virtio-mini device removed\n");
    kfree(dev->priv);
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

/* We don't do anything special at init/exit
 * this replaces module_init()/module_exit() 
 */
module_virtio_driver(driver_virtio_mini);
