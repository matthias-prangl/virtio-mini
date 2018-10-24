#ifndef QEMU_VIRTIO_MINI_H_
#define QEMU_VIRTIO_MINI_H_

#include "standard-headers/linux/virtio_mini.h"

#define TYPE_VIRTIO_MINI "virtio-mini-device"

typedef struct VirtIOMini {
    VirtIODevice parent_obj;
} VirtIOMini;

#endif 