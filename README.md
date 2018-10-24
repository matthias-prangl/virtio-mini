# virtio-mini
virtio example front-end and back-end

## back-end in qemu
virtio-mini is a virtio device that does nothing at all at this point.

Add the device to a running vm with the qemu monitor: `device_add virtio-mini`

If the virtio_mini kernel module is loaded, a kernel info saying the device was found should show up.

## front-end in vm
The virtio_mini kernel module can be used to check if the device is added to the vm.

Just like the virtio-mini device it does nothing...