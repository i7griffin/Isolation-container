# Debugging Commands

## Binary inspection
file ./build/isolation_container
ldd ./build/isolation_container
readelf -l ./build/isolation_container

## Namespace inspection
lsns
readlink /proc/self/ns/pid
readlink /proc/self/ns/mnt
readlink /proc/self/ns/net

## Network inspection
ip addr
ip route

## Mount inspection
findmnt
cat /proc/mounts
