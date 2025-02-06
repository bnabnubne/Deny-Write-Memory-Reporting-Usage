# 🖥️ Deny Writing & Memory Usage Reporting in Pintos

## 📌 Overview
This project, developed as part of the **Operating Systems** course at **Hanoi University of Science and Technology**, focuses on enhancing file system security and memory management in the **Pintos operating system**. Specifically, it implements two key features:

- **Deny Writing to Executables**: Prevents running programs from being modified, enhancing file system integrity and security.
- **Memory Usage Reporting**: Implements system calls for tracking memory allocation and usage in real-time.

## ✨ Features
### 🔒 Deny Writing to Executables
- Prevents modification of executables while they are running.
- Enhances system security by blocking unauthorized file alterations.
- Implements **file_deny_write()** and **file_allow_write()** functions in `file.c`.
- Ensures controlled execution using modifications in `process.c` and `syscall.c`.

### 📊 Memory Usage Reporting
- Tracks **total, free, and used memory pages** in the system.
- Implements system calls to report memory statistics.
- Integrates memory tracking into Pintos' allocation system (`palloc.c`, `init.c`).
- Supports monitoring memory fragmentation and stability.

## 🛠 Technologies Used
- **OS Framework**: Pintos (Stanford’s instructional OS)
- **Programming Language**: C
- **System Calls & Kernel Development**
- **Memory Management & File System Enhancements**

## 🚀 Installation & Setup
### 🔧 Prerequisites
- Install **QEMU** and **Bochs** emulator.
- Set up **Pintos development environment**.

### 📥 Clone and Build the Project
```bash
git clone https://github.com/yourusername/Deny-Write-Memory-Reporting-Usage.git
cd Deny-Write-Memory-Reporting-Usage
make
```

### ▶️ Running the Project
#### Run Deny Writing Feature
```bash
pintos -- run 'deny_write_test'
```

#### Run Memory Usage Reporting
```bash
pintos -- run 'mem_report'
```

## 📝 Key Modifications
### 🔒 Deny Writing to Executables
- `file_deny_write(struct file *file)`: Blocks write operations on executables.
- `file_allow_write(struct file *file)`: Restores write access after process termination.
- `load() (process.c)`: Ensures executables are protected upon loading.
- `sys_write()` in `syscall.c`: Verifies write permissions before executing.

### 📊 Memory Usage Reporting
- `report_memory_usage()`: Retrieves current memory stats (total, used, free pages).
- `syscall.c`: Adds a new syscall `SYS_MEM_REPORT` for memory tracking.
- `palloc.c`: Implements memory statistics functions.
- `tests/report_mem.c`: Validates accuracy of memory tracking.


