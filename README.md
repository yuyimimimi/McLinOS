# Experimental Microkernel OS Project

⚠️ **Project Status**: Actively developed and tested. **For learning/research purposes only**.

## Overview

This is a **fully cross-platform experimental microkernel operating system** featuring:

- Support for multiple processor architectures
- Multiple filesystem support
- Default test platform: **Lichuang XingkongPai** development board

**Project Goal**: Build a portable, configurable, modular microkernel with **Linux kernel space interface compatibility**.

---

## Build Instructions

### Requirements
- Linux terminal environment
- Standard `make` build system

### Build Commands
| Command | Description |
|---------|-------------|
| `make menuconfig` | Graphical kernel configuration |
| `make dtbs` | Device Tree compilation |
| `make` | Standard build |

Additional build capabilities:
- Boot script generation
- Linker script generation
- Interrupt vector table generation

### Setup

1. **Required Tools**:
   - Manually install cross-compiler (e.g., `arm-none-eabi-gcc` - varies by platform)
   
2. **Optional Automated Setup**:
   ```bash
   make installtools




中文说明 (Chinese Version)
本项目仍在积极开发测试中，仅供学习研究使用。

项目特点
完全跨平台微内核操作系统实验项目

支持多种处理器架构与文件系统

默认测试平台：立创星空派开发板

项目目标：构建可移植、可配置、模块化且兼容Linux内核空间的微内核

编译说明
需在Linux终端环境下使用标准make构建系统：

命令	功能
make menuconfig	图形化内核配置
make dtbs	设备树编译
make	标准编译
其他功能：

启动脚本生成

链接器脚本生成

中断向量表生成

环境准备
必需工具：

手动安装交叉编译器（如arm-none-eabi-gcc，不同平台需求不同）

可选自动安装：

bash
make installtools

查看详细帮助：可查看help

