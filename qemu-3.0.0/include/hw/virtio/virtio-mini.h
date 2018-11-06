#ifndef QEMU_VIRTIO_MINI_H_
#define QEMU_VIRTIO_MINI_H_

#include "standard-headers/linux/virtio_mini.h"

#define TYPE_VIRTIO_MINI "virtio-mini-device"
#define VIRTIO_MINI(obj) \
        OBJECT_CHECK(VirtIOMini, (obj), TYPE_VIRTIO_MINI)

typedef struct VirtIOMini {
    VirtIODevice parent_obj;
    VirtQueue *vq_tx;
    VirtQueue *vq_rx;
} VirtIOMini;

#endif 