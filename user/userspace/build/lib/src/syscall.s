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
	.file	"syscall.c"
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
@ 24 "lib/src/syscall.c" 1
	svc 0
@ 0 "" 2
	.thumb
	.syntax unified
	add	sp, sp, #16
	@ sp needed
	pop	{r4, pc}
	.size	__system_call, .-__system_call
	.ident	"GCC: (15:13.2.rel1-2) 13.2.1 20231009"
