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


