	.cpu cortex-m4
	.arch armv7e-m
	.fpu fpv4-sp-d16
	.eabi_attribute 27, 1
	.eabi_attribute 28, 1
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 4
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"main.c"
	.text
	.section	.rodata.str1.1,"aMS",%progbits,1
.LC0:
	.ascii	"/dev/ttyS0\000"
	.section	.text.startup,"ax",%progbits
	.align	1
	.global	main
	.syntax unified
	.thumb
	.thumb_func
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r0, .L4
	push	{r4, lr}
	ldr	r4, .L4+4
	movs	r1, #2
.LPIC0:
	add	r0, pc
	bl	open(PLT)
.LPIC1:
	add	r4, pc
	ldr	r2, .L4+8
	ldr	r2, [r4, r2]
	cmp	r0, #0
	str	r0, [r2, #136]
	blt	.L3
	movs	r4, #0
	strd	r4, r4, [r2, #128]
	bl	close(PLT)
	mov	r0, r4
.L1:
	pop	{r4, pc}
.L3:
	mov	r0, #-1
	b	.L1
.L5:
	.align	2
.L4:
	.word	.LC0-(.LPIC0+4)
	.word	_GLOBAL_OFFSET_TABLE_-(.LPIC1+4)
	.word	shell(GOT)
	.size	main, .-main
	.global	shell
	.bss
	.align	2
	.type	shell, %object
	.size	shell, 268
shell:
	.space	268
	.ident	"GCC: (15:13.2.rel1-2) 13.2.1 20231009"
