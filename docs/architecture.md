# Container Architecture

This project explores the Linux kernel mechanisms behind containerization.

## Components

- Linux namespaces
- Custom root filesystem
- BusyBox userspace
- pivot_root
- Process isolation
- Mount isolation
- Network isolation

## Execution Flow

1. Prepare the root filesystem.
2. Create isolated namespaces.
3. Bind mount the new root filesystem.
4. Switch root using pivot_root.
5. Detach the old root filesystem.
6. Mount procfs.
7. Launch an interactive shell.
