#!/bin/bash
# pid_pivot_isolation.sh — PID namespace + pivot_root demo
# Usage: sudo ./pid_pivot_isolation.sh

echo "[parent] PID on host: $$"

ROOTFS="$(pwd)/myrootfs"

#created network namespace
ip netns add contaniners 

#creating and attaching a virtual ethernet pair 
sudo ip link add vethmain type veth peer name eth0

#moving the container side of the pair into container namespace 
sudo ip link set eth0 netns container

#configuring the econtainer side inside the namespace
sudo ip netns exec container ip addr add 10.0.0.2/24 dev eth0
sudo ip netns exec container ip link set eth0 up


#start the loopback interface
sudo ip netns exec containers ip link set lo up

#configuing the host side 
sudo ip addr add 10.0.0.1/24 dev vethmain
sudo ip link set vethmain up


# ---- Build the minimal rootfs on the host, BEFORE entering the namespace ----
mkdir -p "$ROOTFS/bin"
mkdir -p "$ROOTFS/proc"
mkdir -p "$ROOTFS/oldroot"

# put a static binary here, e.g.:
# gcc -static -o hello hello.c
# cp hello "$ROOTFS/bin/"

# ---- Everything below runs INSIDE the new PID + mount namespace ----
sudo unshare --pid --mount --fork --mount-proc /bin/sh -c "
  echo '[child] PID inside new namespace:' \$\$

  # make the rootfs a mount point (required by pivot_root)
  mount --bind $ROOTFS $ROOTFS

  cd $ROOTFS
  pivot_root . oldroot
  cd /
  umount -l /oldroot
  rmdir /oldroot

  mount -t proc proc /proc

  echo '[child] pivoted. now inside:'
  ls /

  exec /bin/sh
"
