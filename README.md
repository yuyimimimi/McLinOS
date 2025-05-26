这是一个实验性的单片机内核。属于通用操作系统而不是rtos(实际也支持硬实时)

内核设计为跨平台，由于本人时间有限，目前只支持了stm32f407系列

本内核模仿linux内核。提供类unix操作系统的功能

内核空间中你可以使用/linux/xx.h 进行程序的开发。注意内核空间无c语言标准库支持

架构图:
![image](https://github.com/user-attachments/assets/f76c9287-fc10-4d67-b57d-7d0b8404dfc8)




编译步骤：
1.你需要先下载本项目源代码。使用linux终端或wsl打开

2.在根目录下支持``` sudo apt install make``` 下载make工具

3.之后你需要下载自己对应平台的gcc编译器

4.使用``` make installtools ```自动安装其它依赖的程序

5.使用``` make menuconfig ```指令进行配置。进行内存参数配置，你需要配置内存布局，外设中断数量，内核参数，驱动，子系统，调度器等以及许多其它选项

6.你可以进入/user/rootfs/boot 配置config.txt文件。你可以通过它配置加载器以及内核启动的第一个用户程序所在路径

7.你可以将你编译的用户程序放在/user/rootfs/bin目录下

7.你可以进入/arch/$(STRUCT)/boot/dts/$(PLATFORM).dts 配置你所使用平台的设备树文件

8.使用``` make dtbs ```生成并编译设备树

9.执行``` make rootfs ```编译根文件系统，它们默认会被直接嵌入固件，有initramfs管理

10.执行``` make ```编译出最终固件。

用户程序开发环境在

/user/userspace下。目前用户空间的c语言标准库支持并不是很完善，请见谅

你可以修改

/user/rootfs/etc/syscall.tbl 修改系统调用表。它会在内核启动时被动态解析

/user/rootfs/boot/config.txt 用于配置系统的级别启动项，这个文件所在地址是固定的，不要修改它的名称或移动它

/user/rootfs 下的所有文件都会在执行make rootfs 时被透明编译到固件中，默认为initramfs管理，所以你不需要外接存储器就能使用它们。你可以在里面存放任何文件


注意用户空间和内核空间是完全独立的工程，所以你不能混用它们的api。

内核空间遵循linux内核规范，所以你不能在内核空间中使用c语言标准库。而是使用linux/xx.h  asm/xx.h进行开发。

 














