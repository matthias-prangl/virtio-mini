# virtio-mini
virtio example front-end and back-end

For now it is possible to send a predefined message to the hosts qemu monitor by opening the proc file for a *virtio-mini* device.

## back-end in qemu
virtio-mini is a virtio device that receives a buffer from the guest.

Add the device to a running vm with the qemu monitor: `device_add virtio-mini`

## front-end in vm
The virtio_mini kernel module registers a proc file for each virtio-device instance. 
If the proc file is opened a virtqueue buffer is added and sent to the host.
