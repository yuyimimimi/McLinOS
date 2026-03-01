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
	.file	"systemcall.c"
	.text
	.align	1
	.global	syscall
	.syntax unified
	.thumb
	.thumb_func
	.type	syscall, %function
syscall:
	@ args = 4, pretend = 16, frame = 24
	@ frame_needed = 0, uses_anonymous_args = 1
	push	{r0, r1, r2, r3}
	push	{r4, r5, r6, r7, lr}
	sub	sp, sp, #28
	add	r3, sp, #48
	ldr	r0, [sp, #52]
	ldr	r4, [r3], #4
	str	r3, [sp, #4]
	ldrd	r7, r6, [r3, #12]
	ldr	r5, [r3, #20]
	ldrd	r1, r2, [r3, #4]
	strd	r7, r6, [sp, #8]
	strd	r5, r4, [sp, #16]
	add	r3, sp, #8
	.syntax unified
@ 25 "lib/src/systemcall.c" 1
	svc 0
@ 0 "" 2
	.thumb
	.syntax unified
	add	sp, sp, #28
	@ sp needed
	pop	{r4, r5, r6, r7, lr}
	add	sp, sp, #16
	bx	lr
	.size	syscall, .-syscall
	.ident	"GCC: (15:13.2.rel1-2) 13.2.1 20231009"
