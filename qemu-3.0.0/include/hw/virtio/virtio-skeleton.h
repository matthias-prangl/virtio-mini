#ifndef QEMU_VIRTIO_SKELETON_H_
#define QEMU_VIRTIO_SKELETON_H_

typedef struct VirtIOSkeleton {
    VirtIODevice parent_obj;
    VirtQueue *vq;
} VirtIOSkeleton;
#define TYPE_VIRTIO_SKELETON "virtio-skeleton-device"
/* allow casting from VirtIODevice to VirtIOSkeleton */
#define VIRTIO_SKELETON(obj) OBJECT_CHECK(VirtIOSkeleton, (obj), TYPE_VIRTIO_SKELETON)

#endif 