#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/virtio_config.h>
#include "virtio_mini.h"

static int virtio_mini_open(struct inode *inode, struct  file *file) {
    struct virtio_mini_device *vmini = PDE_DATA(inode);
    file->private_data = vmini;
    return 0;
}

static ssize_t virtio_mini_read(struct file *fil, char *buf, size_t count, loff_t *offp) {
    char *rcv_buf; 
    struct scatterlist sg;
    struct virtio_mini_device *vmini = fil->private_data;

    if(vmini->buffers < 1) {
        printk(KERN_INFO "all buffers read!");
        return 0;
    }

    rcv_buf = kzalloc(vmini->buf_lens[vmini->buffers - 1], GFP_KERNEL);
    if(!rcv_buf) {
        return ENOMEM;
    }

    sg_init_one(&sg, rcv_buf, vmini->buf_lens[vmini->buffers - 1]);
    virtqueue_add_inbuf(vmini->vq_rx, &sg, 1, rcv_buf, GFP_KERNEL);
    virtqueue_kick(vmini->vq_rx);

    wait_for_completion(&vmini->data_ready);
    memcpy(buf, vmini->read_data, vmini->buf_lens[vmini->buffers]);
    kfree(rcv_buf);
    return vmini->buf_lens[vmini->buffers];
}

void *to_send;
static ssize_t virtio_mini_write(struct file* fil, const char *buf, size_t count, loff_t *offp) {
    struct scatterlist sg;
    struct virtio_mini_device *vmini = fil->private_data;
    if(vmini->buffers >= VIRTIO_MINI_BUFFERS) {
        printk(KERN_INFO "all buffers used!");
        return ENOSPC;
    }

    to_send = kmalloc(count, GFP_KERNEL);
    if(!to_send) {
        return 1;
    }

    memcpy(to_send, buf, count);
    sg_init_one(&sg, to_send, count);
    vmini->buf_lens[vmini->buffers++] = count;

    virtqueue_add_outbuf(vmini->vq_tx, &sg, 1, to_send, GFP_KERNEL);
    virtqueue_kick(vmini->vq_tx);
    return count;
}

/* host has acknowledged the message; consume buffer */
void virtio_mini_outbuf_cb(struct virtqueue *vq) {
    int len;
    virtqueue_get_buf(vq, &len);
    kfree(to_send);
    return;
}

void virtio_mini_inbuf_cb(struct virtqueue *vq) {
    int len;
    struct virtio_mini_device *vmini = vq->vdev->priv;
    vmini->read_data = virtqueue_get_buf(vq, &len);
    vmini->buffers--;
    complete(&vmini->data_ready);
    printk(KERN_INFO "Received %i bytes", len);
}

/* assign one virtqueue for transmitting and one for receiving data */
int virtio_mini_assign_virtqueue(struct virtio_mini_device *vmini) {
    const char *names[] = { "virtio-mini-tx", "virtio-mini-rx" };
    vq_callback_t *callbacks[] = { virtio_mini_outbuf_cb, virtio_mini_inbuf_cb };
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
    /* virtio-mini(12) + bus index (7) */
    char proc_name[20];
    int err;
    printk(KERN_INFO "virtio-mini device found\n");

    vmini = kzalloc(sizeof(struct virtio_mini_device), GFP_KERNEL);
    if(vmini == NULL) {
        err = ENOMEM;
        goto err;
    }

    /* make it possible to access underlying virtio_device 
    from virtio_mini_device and vice versa */
    vdev->priv = vmini;
    vmini->vdev = vdev;
    err = virtio_mini_assign_virtqueue(vmini);
    if(err) {
        printk(KERN_INFO "Error adding virtqueue\n");
        goto err;
    }
    vmini->buffers = 0;

    init_completion(&vmini->data_ready);

    /* create a proc entry named "virtio-mini-<bus_idx>" 
    proc_dir_entry data pointer points to associated virtio_mini_device 
    allows access to virtqueues from defined file_operations functions */
    snprintf(proc_name, sizeof(proc_name), "%s-%i", VIRTIO_MINI_STRING, vdev->index);
    vmini->pde = proc_create_data(proc_name, 0644, NULL, &pde_fops, vmini);
    if(!vmini->pde) {
        printk(KERN_INFO "Error creating proc entry");
        goto err;
    }

    return 0;

    err:
    kfree(vmini);
    return err;
}

void remove_virtio_mini (struct virtio_device *vdev) {
    struct virtio_mini_device *vmini = vdev->priv;
    proc_remove(vmini->pde);
    complete(&vmini->data_ready);
    vdev->config->reset(vdev);
    vdev->config->del_vqs(vdev);
    kfree(vdev->priv);
    printk(KERN_INFO "virtio-mini device removed\n");
}

/* We don't do anything special at init/exit
 * this replaces module_init()/module_exit() 
 */
module_virtio_driver(driver_virtio_mini);

MODULE_AUTHOR("Matthias Prangl");
MODULE_DESCRIPTION("virtio example front-end driver");
MODULE_LICENSE("GPL v2");