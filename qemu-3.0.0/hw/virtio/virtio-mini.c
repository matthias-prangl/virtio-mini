#include "qemu/osdep.h"
#include "hw/hw.h"
#include "hw/virtio/virtio.h"
#include "hw/virtio/virtio-mini.h"
#include "qemu/iov.h"
#include "qemu/error-report.h"
#include "standard-headers/linux/virtio_ids.h"

void GCC_FMT_ATTR(1, 2) virtio_mini_print(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    error_vreport(fmt, ap);
    va_end(ap);
}

static uint64_t virtio_mini_get_features(VirtIODevice *vdev, uint64_t f, Error **errp)
{
    return f;
}

static void virtio_mini_set_status(VirtIODevice *vdev, uint8_t status)
{
    if (!vdev->vm_running) {
        return;
    }
    vdev->status = status;
}

/* callback for receiving virtqueue (outbuf on guest) */
static void virtio_mini_handle_outbuf(VirtIODevice *vdev, VirtQueue *vq) {
    /* acllocation of VirtQueueElement happens in virtqueue_pop call */
    VirtIOMini *vmini = VIRTIO_MINI(vdev);
    VirtQueueElement *vqe;

    while(!virtio_queue_ready(vq)) {
        virtio_mini_print("not ready");
        return;
    }
    if (!runstate_check(RUN_STATE_RUNNING)) {
        virtio_mini_print("not synced");
        return;
    }

    vqe = virtqueue_pop(vq, sizeof(VirtQueueElement));
    vmini->rcv_bufs[vmini->rcv_count] = malloc(vqe->out_sg->iov_len);
    iov_to_buf(vqe->out_sg, vqe->out_num, 0, vmini->rcv_bufs[vmini->rcv_count], vqe->out_sg->iov_len);
    virtio_mini_print("received: %s", vmini->rcv_bufs[vmini->rcv_count]);
    vmini->rcv_count++;
    virtqueue_push(vq, vqe, 0);
    virtio_notify(vdev, vq);
    g_free(vqe);
    return;
}

/* callback for transmitting virtqueue (inbuf on guest) */
static void virtio_mini_handle_inbuf(VirtIODevice *vdev, VirtQueue *vq) {
    VirtIOMini *vmini = VIRTIO_MINI(vdev);
    VirtQueueElement *vqe;
    unsigned int last_buf = vmini->rcv_count - 1;

    while(!virtio_queue_ready(vq)) {
        virtio_mini_print("not ready");
        return;
    }
    if (!runstate_check(RUN_STATE_RUNNING)) {
        virtio_mini_print("not synced");
        return;
    }
    vqe = virtqueue_pop(vq, sizeof(VirtQueueElement));

    iov_from_buf(vqe->in_sg, vqe->in_num, 0, vmini->rcv_bufs[last_buf], strlen(vmini->rcv_bufs[last_buf]));
    vmini->rcv_count--;
    virtqueue_push(vq, vqe, vqe->in_sg->iov_len);
    virtio_notify(vdev, vq);
    g_free(vqe);
    return;
}

static void virtio_mini_device_realize(DeviceState *dev, Error **errp) {
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOMini *vmin = VIRTIO_MINI(dev);
    virtio_init(vdev, "virtio-mini", VIRTIO_ID_MINI, 0);
    vmin->vq_rx = virtio_add_queue(vdev, 8, virtio_mini_handle_outbuf);
    vmin->vq_tx = virtio_add_queue(vdev, 8, virtio_mini_handle_inbuf);
    vmin->rcv_bufs = malloc(8 * sizeof(char*));
    vmin->rcv_count = 0;
}

static void virtio_mini_device_unrealize(DeviceState *dev, Error **errp) {
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    virtio_cleanup(vdev);
}

static void virtio_mini_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    VirtioDeviceClass *vdc = VIRTIO_DEVICE_CLASS(klass);

    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
    vdc->realize = virtio_mini_device_realize;
    vdc->unrealize = virtio_mini_device_unrealize;
    vdc->get_features = virtio_mini_get_features;
    vdc->set_status = virtio_mini_set_status;
}

static const TypeInfo virtio_mini_info = {
    .name = TYPE_VIRTIO_MINI,
    .parent = TYPE_VIRTIO_DEVICE,
    .instance_size = sizeof(VirtIOMini),
    .class_init = virtio_mini_class_init,
};

static void virtio_register_types(void) {
    type_register_static(&virtio_mini_info);
}

type_init(virtio_register_types);