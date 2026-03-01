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
.LC1:
	.ascii	"\012\000"
	.section	.text.startup,"ax",%progbits
	.align	1
	.global	main
	.syntax unified
	.thumb
	.thumb_func
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 32
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, lr}
	ldr	r0, .L5
	ldr	r5, .L5+4
	ldr	r7, .L5+8
	sub	sp, sp, #36
	movs	r1, #2
.LPIC0:
	add	r0, pc
	bl	open(PLT)
.LPIC1:
	add	r5, pc
	ldr	r3, .L5+12
	ldr	r3, [r5, r3]
	str	r3, [sp, #4]
	movs	r2, #12
	mov	r1, r3
	mov	r4, r0
	bl	write(PLT)
	add	r6, sp, #12
.LPIC2:
	add	r7, pc
.L2:
	movs	r2, #1
	mov	r1, r6
	mov	r0, r4
	bl	read(PLT)
	cmp	r0, #1
	mov	r5, r0
	bne	.L2
	mov	r2, r0
	mov	r1, r6
	mov	r0, r4
	bl	write(PLT)
	mov	r2, r5
	mov	r1, r7
	mov	r0, r4
	bl	write(PLT)
	b	.L2
.L6:
	.align	2
.L5:
	.word	.LC0-(.LPIC0+4)
	.word	_GLOBAL_OFFSET_TABLE_-(.LPIC1+4)
	.word	.LC1-(.LPIC2+4)
	.word	data(GOT)
	.size	main, .-main
	.global	data
	.data
	.type	data, %object
	.size	data, 12
data:
	.ascii	"hello_test\012\000"
	.ident	"GCC: (15:13.2.rel1-2) 13.2.1 20231009"
