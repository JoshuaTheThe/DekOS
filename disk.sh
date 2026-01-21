#!/bin/bash

SIZE=64M
IMAGE=fat32.img
dd if=/dev/zero of=$IMAGE bs=1M count=64
mkfs.vfat -F 32 -n SLAVE_DISK $IMAGE
