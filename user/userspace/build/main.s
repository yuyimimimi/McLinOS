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
	.align	1
	.global	__system_call
	.syntax unified
	.thumb
	.thumb_func
	.type	__system_call, %function
__system_call:
	@ args = 12, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r3, r4, lr}
	mov	r4, r0
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	ldr	r3, [sp, #24]
	str	r3, [sp]
	ldr	r3, [sp, #28]
	str	r3, [sp, #4]
	ldr	r3, [sp, #32]
	strd	r3, r4, [sp, #8]
	mov	r3, sp
	.syntax unified
@ 61 "main.c" 1
	svc 0
@ 0 "" 2
	.thumb
	.syntax unified
	add	sp, sp, #16
	@ sp needed
	pop	{r4, pc}
	.size	__system_call, .-__system_call
	.align	1
	.global	pr_info
	.syntax unified
	.thumb
	.thumb_func
	.type	pr_info, %function
pr_info:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r3, r4, lr}
	movs	r3, #0
	mov	r1, r0
	strd	r3, r3, [sp, #4]
	str	r3, [sp]
	mov	r2, r3
	mov	r0, r3
	bl	__system_call(PLT)
	add	sp, sp, #20
	@ sp needed
	ldr	pc, [sp], #4
	.size	pr_info, .-pr_info
	.section	.rodata.str1.1,"aMS",%progbits,1
.LC0:
	.ascii	"Hello from user app!\012\000"
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
	push	{r3, r4, r5, lr}
	ldr	r5, .L6
	movs	r4, #10
.LPIC0:
	add	r5, pc
.L4:
	mov	r0, r5
	bl	pr_info(PLT)
	subs	r4, r4, #1
	bne	.L4
	mov	r0, r4
	pop	{r3, r4, r5, pc}
.L7:
	.align	2
.L6:
	.word	.LC0-(.LPIC0+4)
	.size	main, .-main
	.text
	.align	1
	.global	reset_handler
	.syntax unified
	.thumb
	.thumb_func
	.type	reset_handler, %function
reset_handler:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	b	main(PLT)
	.size	reset_handler, .-reset_handler
	.global	app_meta
	.section	.appmeta,"aw"
	.align	2
	.type	app_meta, %object
	.size	app_meta, 32
app_meta:
	.word	12345678
	.word	0
	.word	0
	.word	7
	.word	4096
	.word	reset_handler
	.word	_got_start
	.word	_got_end
	.ident	"GCC: (15:13.2.rel1-2) 13.2.1 20231009"
