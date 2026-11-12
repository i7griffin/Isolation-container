# Building a Linux Container From Scratch

A hands-on exploration of Linux container internals by building a minimal container environment using native Linux kernel features, without Docker.

The objective of this project is to understand how containerization works internally by implementing its fundamental mechanisms manually.

## 1. Project Overview

Containers are not virtual machines. They rely on Linux kernel features such as namespaces, filesystem isolation, and resource controls to create isolated environments for processes.

Instead of relying on Docker or an existing container runtime, this project explores the underlying Linux primitives and combines them to build a minimal container environment.

The implementation currently uses:

- Linux namespaces for isolation.
- BusyBox for a minimal userspace environment.
- `pivot_root` for changing the root filesystem.
- `mount` operations for filesystem isolation.
- A custom executable to test process execution inside the container.

## 2. Development Environment

| Component | Details |
|---|---|
| Host OS | Kali Linux |
| Shell | Bash |
| Architecture | x86-64 |
| Userspace | BusyBox |
| Container filesystem | Custom root filesystem |
| Isolation mechanism | Linux namespaces |
| Programming language | C |
| Container launcher | Shell script / Linux commands |

## 3. Current Root Filesystem

A custom root filesystem named `myrootfs` was created.

```text
container/
│
├── myrootfs/
│   │
│   ├── bin/
│   │   ├── busybox
│   │   ├── hello
│   │   └── hello.c
│   │
│   ├── lib64/
│   │
│   ├── oldroot/
│   │
│   ├── proc/
│   │
│   └── usr/
│
└──
```

### BusyBox

BusyBox provides a collection of essential Linux utilities through a single executable.

It is used to provide a minimal userspace environment without installing a complete Linux distribution.

The BusyBox binary was installed in:

```bash
myrootfs/bin/busybox
```

Its available applets can be inspected using:

```bash
/bin/busybox --list
```

Symbolic links are used to expose BusyBox applets as individual commands.

An issue was encountered with absolute symbolic links after changing the root filesystem using `pivot_root`. This demonstrated the importance of relative symbolic links when constructing a relocatable root filesystem.

## 4. Custom Executable

A small C program named `hello` was created to verify that custom applications could execute inside the container.

The executable was placed at:

```text
myrootfs/bin/hello
```

The program prints a message and its process ID, then sleeps to allow the environment to be inspected.

It was successfully executed inside the new root filesystem.

The observed output included:

```text
Hello from inside the container! PID: 1
Sleeping so you can inspect the environment...
```

This confirmed that the custom executable could run as PID 1 inside the newly created PID namespace.

## 5. Linux Namespaces

The container is currently launched using:

```bash
sudo unshare --mount --pid --fork --net
```

The following namespaces have been introduced.

### Mount Namespace

The mount namespace provides an isolated view of mount points.

The mount propagation settings are changed using:

```bash
mount --make-rprivate /
```

The custom root filesystem is bind-mounted:

```bash
mount --bind "$ROOTFS" "$ROOTFS"
```

This allows mount operations to be isolated from the host mount namespace.

### PID Namespace

The PID namespace provides an independent process ID hierarchy.

It was created using:

```bash
--pid --fork
```

The custom `hello` program was observed running as PID 1 inside the container.

From the host, the same process appeared with a different PID.

This demonstrated how a single process can have different PID values depending on the PID namespace from which it is observed.

### Network Namespace

The network namespace was introduced using:

```bash
--net
```

This provides an isolated network stack for the container.

The container does not automatically inherit the host's network connectivity.

Virtual Ethernet interfaces, routing and NAT have not yet been configured.

## 6. Changing the Root Filesystem Using pivot_root

One of the most important milestones was switching the process into the custom root filesystem.

The operation uses:

```bash
/bin/busybox pivot_root . oldroot
```

The general sequence is:

1. Bind-mount the custom root filesystem.
2. Change the working directory to the new root.
3. Execute `pivot_root`.
4. Move into the new root directory.
5. Unmount the old root filesystem.
6. Remove the old root mount-point directory.

The old root is removed using:

```bash
/bin/busybox umount -l /oldroot
/bin/busybox rmdir /oldroot
```

The `oldroot` directory must exist before calling `pivot_root`.

This operation changes the root filesystem visible to the process, rather than merely changing its working directory.

## 7. Mounting procfs

After changing the root filesystem, `/proc` is mounted:

```bash
/bin/busybox mount -t proc proc /proc
```

This provides access to process and kernel information through the proc filesystem.

It is particularly important when inspecting processes inside the PID namespace.

## 8. Interactive Container Shell

Initially, the container executed the custom `hello` program and terminated after its execution.

To make the environment interactive, the final execution command was changed to:

```bash
exec /bin/busybox sh
```

This launches the BusyBox shell inside the custom root filesystem.

The container now provides an interactive prompt:

```text
/ #
```

This allows commands to be executed directly inside the isolated environment.

## 9. Issues Encountered and Debugging

Several problems were encountered during implementation.

### BusyBox symbolic link failures

Initially, BusyBox applets were installed using:

```bash
busybox --install -s /bin
```

The generated links contained absolute paths pointing to the host filesystem.

After `pivot_root`, those paths were no longer accessible inside the new root filesystem.

The solution identified was to use relative symbolic links pointing to the BusyBox executable.

### Missing commands

Even after launching the BusyBox shell, several commands were unavailable.

This happened because the custom root filesystem did not contain a complete Linux userspace.

BusyBox applets must be available through the executable or their corresponding symbolic links.

### pivot_root failures

The operation initially failed because the `oldroot` mount-point directory did not exist.

Creating the directory before executing `pivot_root` resolved this issue.

### Dynamically linked executables

Inspection using `file` and `ldd` showed that BusyBox and the custom `hello` executable were dynamically linked.

Their required dynamic linker and shared libraries therefore need to be available inside the custom root filesystem.

This introduced another important aspect of constructing a functional root filesystem: executable files alone are not always sufficient.

## 10. Current Implementation Status

The following functionality has been implemented and tested:

- Custom root filesystem creation.
- BusyBox integration.
- Custom C executable.
- Mount namespace creation.
- PID namespace creation.
- Network namespace creation.
- Bind mounting the custom root filesystem.
- Root filesystem switching using `pivot_root`.
- Removal of the old root mount.
- Mounting procfs.
- Executing a custom program inside the container.
- Running an interactive BusyBox shell.

The BusyBox command links and availability of individual applets still require further debugging.

## 11. Current Limitations

The implementation is still a minimal container prototype.

It does not yet implement:

- Cgroup-based CPU and memory limits.
- User namespace isolation and UID/GID mapping.
- UTS namespace isolation.
- IPC namespace isolation.
- Virtual Ethernet networking.
- Bridge networking and NAT.
- Linux capability restrictions.
- Seccomp filtering.
- Container lifecycle management.
- Automated root filesystem preparation.

These features represent further stages of development rather than completed functionality.

## 12. Project Direction

The long-term objective is to develop a simplified Linux container runtime that demonstrates how containerization works internally.

The next areas of development include:

1. Completing the BusyBox userspace.
2. Improving interactive container management.
3. Configuring virtual Ethernet interfaces.
4. Establishing communication between the host and container.
5. Implementing cgroups v2 for resource control.
6. Exploring user namespaces and UID/GID mapping.
7. Implementing capability restrictions and seccomp.
8. Combining the individual components into a reusable container launcher.

## 13. Learning Objectives

This project explores the relationship between Linux kernel primitives and containerization.

It provides practical exposure to:

- Linux process management.
- PID namespace behavior.
- Mount propagation and filesystem isolation.
- Root filesystem construction.
- Dynamic linking and shared libraries.
- Network namespace isolation.
- Linux system calls and kernel interfaces.
- Container runtime architecture.

The emphasis is on understanding the underlying mechanisms rather than depending on high-level containerization tools.

---

**Project status:** Minimal container prototype with an interactive BusyBox shell and initial namespace isolation implemented.

**Primary objective:** Understand and implement the fundamental mechanisms behind Linux containers from scratch.
