# Root Filesystem

The project uses a minimal custom root filesystem.

## Components

- /bin: Executables and BusyBox
- /lib64: Dynamic linker
- /usr/lib: Shared libraries
- /proc: Process information
- /oldroot: Temporary pivot_root mount point

The populated root filesystem is generated locally and excluded from Git.
