# Isolation-container
# mini-container-runtime

A minimal container runtime built from scratch in C — the core isolation mechanics behind Docker, without the images, daemon, or orchestration.

## What it does

Runs a process in isolation using:

- **Namespaces** — PID, mount, and network namespaces so the process can't see the host's processes, filesystem, or network
- **cgroups (v2)** — limits on memory and process count so it can't hog resources
- **seccomp** — a syscall filter that blocks dangerous syscalls

## Status

🚧 Work in progress — building milestone by milestone.

- [ ] PID namespace isolation
- [ ] Mount namespace + rootfs swap (pivot_root)
- [ ] Network namespace + veth pair
- [ ] cgroup resource limits
- [ ] seccomp syscall filtering

## Usage (planned)

```
./myruntime <rootfs-path> <command>
```

## Why

Built to understand what's actually happening under `docker run` — namespaces, cgroups, and seccomp at the syscall level.



 --- after isolating the child process from the parent process , we need to give or mount the child process a fake root directory and file system 
 --- for that we need a minimal root file system to install and mount it for the child process we isolated to fake it by giving it its own environment
