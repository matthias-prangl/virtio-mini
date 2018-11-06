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

struct virtio_mini_device {
    /* receiving virtqueue */
    struct virtqueue *vq_rx;
    /* transmitting virtqueue */
    struct virtqueue *vq_tx;
    /* related virtio_device */
    struct virtio_device *vdev;
};

void virtio_mini_vq_tx_cb(struct virtqueue *vq) {
    printk(KERN_INFO "tx callback !\n");
}

void virtio_mini_vq_rx_cb(struct virtqueue *vq) {
    printk(KERN_INFO "rx callback !\n");
}

int virtio_mini_assign_virtqueue(struct virtio_mini_device *vmini) {
    const char *names[] = { "virtio-mini-tx", "virtio-mini-rx" };
    vq_callback_t *callbacks[] = { virtio_mini_vq_tx_cb, virtio_mini_vq_rx_cb };
    struct virtqueue *vqs[2];
    int err;

    err = virtio_find_vqs(vmini->vdev, 2, vqs, callbacks, names, NULL);
    if(err) {
        return err;
    }
    vmini->vq_tx = vqs[0];
    vmini->vq_rx = vqs[1];
    return 0;
}

int probe_virtio_mini(struct virtio_device *vdev) {
    struct virtio_mini_device *vmini;
    int err;

    printk(KERN_INFO "virtio-mini device found\n");

    vmini = kzalloc(sizeof(struct virtio_mini_device), GFP_KERNEL);
    if(vmini == NULL) {
        err = ENOMEM;
        goto err;
    }
    vdev->priv = vmini;
    vmini->vdev = vdev;

    err = virtio_mini_assign_virtqueue(vmini);
    if(err) {
        printk(KERN_INFO "Error adding virtqueue\n");
        goto err;
    }
    printk(KERN_INFO "tx queue: %s \t rx queue: %s\n", 
        vmini->vq_tx->name, vmini->vq_rx->name);
    
    return 0;

    err:
    kfree(vmini);
    return err;
}

void remove_virtio_mini (struct virtio_device *vdev) {
    printk(KERN_INFO "virtio-mini device removed\n");
    vdev->config->reset(vdev);
    vdev->config->del_vqs(vdev);
    kfree(vdev->priv);
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
