This project is still under active development and testing. It is intended **only for learning, research, and experimentation by hobbyists**.

This is a **fully cross-platform experimental microkernel operating system** project. It supports multiple processor architectures and file systems. The default test platform uses the **Lichuang XingkongPai** development board.

The goal is to build a **portable, configurable, modular microkernel** that is **compatible with the Linux kernel space interface**.

---

## Build System

The project must be compiled in a **Linux terminal environment** using the standard `make` build system.

- Supports graphical kernel configuration using `make menuconfig`
- Supports Device Tree — compile with:
  ```bash
  make dtbs





本项目仍在开发与测试中，仅供学习、研究和爱好者尝试使用。

这是一个完全跨平台的微内核操作系统实验项目，支持多种处理器架构与文件系统，默认测试平台使用 **立创星空派** 开发板

目标是构建一个可移植、可配置、模块化且与linux内核空间兼容的操作系统内核

项目需在 Linux 终端下编译，使用标准 `make` 构建系统：

可通过 `make menuconfig` 图形化配置内核选项
支持设备树 使用'make dtbs'命令进行编译     
可进行启动脚本,链接器脚本,中断向量表的生成

###  前置准备
1. **手动安装交叉编译器**（如 `arm-none-eabi-gcc`，因不同平台需求各异）
2. 自动安装所需工具链（可选）：
   ```bash
   make installtools

你可以查看help文件获得具体帮助


