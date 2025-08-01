# 0 "/mnt/c/Users/31740/Desktop/newcore/arch/arm_m/boot/dts/stm32f4.dts"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/mnt/c/Users/31740/Desktop/newcore/arch/arm_m/boot/dts/stm32f4.dts"
/dts-v1/;

/ {
    model = "STM32F407 Board";
    compatible = "st,stm32f407";

    soc {
        #address-cells = <1>;
        #size-cells = <1>;

        intc: interrupt-controller@e000e100 {
            compatible = "arm,cortex-m4-nvic";
            interrupt-controller;
            #interrupt-cells = <1>;
            reg = <0xe000e100 0x400>;
        };

        uart1: serial@40011000 {
            compatible = "st,stm32-uart";
            reg = <0x40011000 0x400>;
            interrupts = <37>;
            interrupt-parent = <&intc>;
            current-speed = <115200>;
            status = "okay";
        };
    };

};
