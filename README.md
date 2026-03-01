McLinOS: A Linux-Compatible Micro-Kernel for MCUs

McLinOS is an experimental, cross-platform operating system kernel designed for Microcontroller Units (MCUs).  McLinOS is architected as a General-Purpose Operating System (GPOS) while maintaining deterministic hard real-time capabilities.The core philosophy of McLinOS is to bridge the gap between RTOS and Embedded Linux, enabling "Write Once, Run Anywhere" by providing a native Linux-compatible kernel environment on resource-constrained hardware.


 [!CAUTION] This is an experimental project and is not recommended for production environments.

 1.Key Features Architecture Support: STM32F407 (Cortex-M4), STM32H743 (Cortex-M7), and RP2350 (Cortex-M33). 
 
 2.Linux API Compatibility: Provides ~150 standard Linux 5.4 kernel headers. Develop drivers using standard linux/xx.h and asm/xx.h patterns.
 
 3.Unix-like Subsystems: Virtual File System (VFS), devfs, initramfs, and Unix-style process management.

 4.Decoupled Environments: Strict separation between Kernel Space (No Libc) and User Space (Standard Libc supported).


这是一个实验性的单片机内核。属于通用操作系统而不是rtos(实际也支持硬实时)，不建议用于生产环境

内核设计为跨平台

支持了stm32f407 stm32h743 rp2350

本内核模仿了linux内核空间以及开发环境。提供类unix操作系统的功能。并且提供真正的linux内核空间api。

目标为让代码一次编写，多处运行，不再需要区分rtos和嵌入式linux。



System Architecture:
架构图:
<img width="865" height="698" alt="image" src="https://github.com/user-attachments/assets/5122d652-739e-4cc8-a6a3-22a34c6289eb" />



内核空间中你可以使用/linux/xx.h 进行程序的开发。注意内核空间无c语言标准库支持

用户程序可以使用c语言标准库，但是必须单独编译为独立app。

 Build Instructions

1. Prerequisites

Ensure you are using a Linux environment (Native or WSL).
```bash
sudo apt update && sudo apt install make
# Install the cross-compiler for your target (e.g., arm-none-eabi-gcc)
```

2. Configuration
Modify the root Makefile to match your hardware target:

```makefile
STRUCT    = arm_m
PLATFORM  = stm32f4  # or stm32h7, rp2350
PREFIX    = arm-none-eabi-
VENDOR    = st       # or raspberrypi
```
3. Compilation Workflow
   
1.Install Toolchain Dependencies: make installtools

2.Kernel Configuration: Run make menuconfig to configure memory layout, interrupt vectors, scheduler parameters, and drivers.

3.Device Tree Setup: Edit /arch/$(STRUCT)/boot/dts/$(PLATFORM).dts and compile via make dtbs.

4.RootFS Preparation:

    Place user apps in /user/rootfs/bin.

    Configure boot parameters in /user/rootfs/boot/config.txt.

    Build the image: make rootfs (Files are transparently embedded into firmware via initramfs).

5.Final Build: Execute make to generate the firmware image.

6.Enter the bootloader directory, navigate to the corresponding device name, and use the make command to compile the bootloader.


编译步骤：

1.你需要先下载本项目源代码。使用linux终端或wsl打开

2.在根目录下支持``` sudo apt install make``` 下载make工具

3.之后你需要下载自己对应平台的gcc编译器

4.打开根makefile配置参数，例如: 
STRUCT    = arm_m
PLATFORM  = rp2350
PREFIX    = arm-none-eabi-
VENDOR    = raspberrypi

5.使用``` make installtools ```自动安装其它依赖的程序

6.使用``` make menuconfig ```指令进行配置。进行内存参数配置，你需要配置内存布局，外设中断数量，内核参数，驱动，子系统，调度器等以及许多其它选项

7.你可以进入/user/rootfs/boot 配置config.txt文件。你可以通过它配置加载器以及内核启动的第一个用户程序所在路径

8.你可以将你编译的用户程序放在/user/rootfs/bin目录下

9.你可以进入/arch/$(STRUCT)/boot/dts/$(PLATFORM).dts 配置你所使用平台的设备树文件

10.使用``` make dtbs ```生成并编译设备树

11.执行``` make rootfs ```编译根文件系统，它们默认会被直接嵌入固件，有initramfs管理

12.执行``` make ```编译出最终固件。

13.进入bootloader目录旋转对应的设备名称，使用make命令编译出bootloader


Development Guide

Kernel Space

    Standards: Follows Linux Kernel coding standards.

    Headers: Use <linux/xx.h> or <asm/xx.h>.

    Constraint: Standard C libraries (libc) are prohibited in kernel space.

User Space

    Location: Source code resides in /user/userspace.

    Features: Supports standard libc (currently under development).


用户程序开发环境在

/user/userspace下。目前用户空间的c语言标准库支持并不是很完善，请见谅

你可以修改

/user/rootfs/etc/syscall.tbl 修改系统调用表。它会在内核启动时被动态解析

/user/rootfs/boot/config.txt 用于配置系统的启动项，这个文件所在地址是固定的，不要修改它的名称或移动它

/user/rootfs 下的所有文件都会在执行make rootfs 时被透明编译到固件中，默认为initramfs管理，所以你不需要外接存储器就能使用它们。你可以在里面存放任何文件

注意用户空间和内核空间是完全独立的工程，所以你不能混用它们的api。

内核空间遵循linux内核规范，所以你不能在内核空间中使用c语言标准库。而是使用linux/xx.h  asm/xx.h ... 。

我提供了大约150个linux内核中的标准的头文件和linux5.4兼容，你可以使用它们，并使用开发linux内核模块的方式和习惯开发内核，你需要将它当作linux开发内核模块。

 
效果：
<img width="829" height="2113" alt="image" src="https://github.com/user-attachments/assets/a69293c5-0b19-4d53-94c2-986ed71021ae" />














