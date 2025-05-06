this RTOS kernel just for make compatibility of linux kernel.
It's just for personal test.

I'm not recommend any one use it on any project.

no libc no MDK support.

you need install make tools。

and use cmd:"make installtools"

then install gcc for your platforme

use make menuconfig to config kernel.
then link script and boot script will be create 

use make dtbs to create device tree

use make 

you will find file at./build/out/

