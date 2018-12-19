#include "qemu/osdep.h"
#include "qemu/iov.h"

#include "hw/virtio/virtio.h"
#include "hw/virtio/virtio-skeleton.h"
#include "standard-headers/linux/virtio_ids.h"

/* virtqueue size must be a power of 2, max. 2^15 */
#define QUEUE_SIZE 128

/* minimal VIRTIO Device with one virtqueue */


/* called when the queue is "kicked" by the driver.
   Don't forget checks and releasing of resources! */
static void skeleton_callback(VirtIODevice *vdev, VirtQueue *vq) {
    VirtQueueElement *vqe;
    void *buf;
    /* get the next element from the virtqueue. Allocation happens in virtqueue_pop! */
    vqe = virtqueue_pop(vq, sizeof(VirtQueueElement));
    /* vqe contains scatter-gather-lists (iov in qemu) either for in- or output or for both.
       Here one output iov is assumed */
    buf = malloc(vqe->out_sg->iov_len);
    iov_to_buf(vqe->out_sg, vqe->out_num, 0, buf, vqe->out_sg->iov_len);
    /* signal the front-end driver that an element has been used */
    virtqueue_push(vq, vqe, 0);
    virtio_notify(vdev, vq);
}

/* used to add a device to the vm (device_add in qemu-monitor) */
static void skeleton_realize(DeviceState *dev, Error **errp) {
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOSkeleton *vskel = VIRTIO_SKELETON(dev);
    /* initialize a virtio device with config space of size 0 */
    virtio_init(vdev, TYPE_VIRTIO_SKELETON, VIRTIO_ID_SKELETON, 0);
    /*  add a virtqueue to the device. 
        skeleton_callback is called after the front-end driver "kicked" the queue */
    vskel->vq = virtio_add_queue(vdev, QUEUE_SIZE, skeleton_callback);
}

/* used to remove a device from the vm (device_del in qemu-monitor) */
static void skeleton_unrealize(DeviceState *dev, Error **errp) {
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    virtio_cleanup(vdev);
}

/* the front end driver uses this function to get the features of the device as a bit-field */
static uint64_t skeleton_get_features(VirtIODevice *vdev, uint64_t features, Error **errp) {
    return features;
}

/* status bits are set by the front end driver */
static void skeleton_set_status(VirtIODevice *vdev, uint8_t status) {
    vdev->status = status;
}

static void skeleton_class_init(ObjectClass *oc, void *data) {
    DeviceClass *dc = DEVICE_CLASS(oc);
    VirtioDeviceClass *vdc = VIRTIO_DEVICE_CLASS(oc);
    
    /* set device category according to include/hw/qdev-core.h DeviceCategory */
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);

    /* at least these functions have to be implemented */
    vdc->realize = skeleton_realize;
    vdc->unrealize = skeleton_unrealize;
    vdc->get_features = skeleton_get_features;
    vdc->set_status = skeleton_set_status;
}

static const TypeInfo skeleton_info = {
    .name = TYPE_VIRTIO_SKELETON,
    .parent = TYPE_VIRTIO_DEVICE,
    .instance_size = sizeof(VirtIOSkeleton),
    .class_init = skeleton_class_init,
};

static void register_types(void) {
    type_register_static(&skeleton_info);
}

type_init(register_types);