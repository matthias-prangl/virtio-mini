#include "qemu/osdep.h"
#include "hw/hw.h"
#include "hw/virtio/virtio.h"
#include "hw/virtio/virtio-mini.h"
#include "standard-headers/linux/virtio_ids.h"

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

static void virtio_mini_handle_input(VirtIODevice *vdev, VirtQueue *vq) {
    /* do nothing */
}

static void virtio_mini_handle_output(VirtIODevice *vdev, VirtQueue *vq) {
    /* do nothing */
}

static void virtio_mini_device_realize(DeviceState *dev, Error **errp) {
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOMini *vmin = VIRTIO_MINI(dev);
    virtio_init(vdev, "virtio-mini", VIRTIO_ID_MINI, 0);
    vmin->vq_tx = virtio_add_queue(vdev, 8, virtio_mini_handle_input);
    vmin->vq_rx = virtio_add_queue(vdev, 8, virtio_mini_handle_output);
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