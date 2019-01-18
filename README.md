# virtio-mini
VIRTIO example front-end and back-end.

*virtio-mini* serves mainly as a simple demonstration on how to use VIRTIO Devices in QEMU. 
The implementation enables the usage of a character device (`/proc/virtio-mini-n`) in the guest to send data to a stack-like data structure on the host.
The data sent to the host can then be read again by the guest using the same character device.

## back-end in qemu
virtio-mini is a virtio device that receives a buffer from the guest.

Add the device to a running vm with the qemu monitor: `device_add virtio-mini`

## front-end in vm
The virtio_mini kernel module registers a proc file for each virtio-device instance. 

## Building QEMU with virtio-mini

Enable the *virtio-mini* option to build the device alongside QEMU: `--enable-virtio_mini`