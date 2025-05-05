this RTOS kernel just create extreme compatibility of linux kernel.
I's just for personal test.
I'm not recommend any one use it for any project.


no libc no MDK support.

and now it's only have stm32f407 support.

you need instell make tools。
and use cmd: "make installtools"
then install arm-none-eabi-gcc

use make menuconfig to config kernel.
then link script and boot script will be create 

use make dtbs to create device tree

use make 

you will find file at./build/out/
