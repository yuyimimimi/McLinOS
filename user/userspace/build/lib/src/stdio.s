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
	.file	"stdio.c"
	.text
	.align	1
	.global	_open
	.syntax unified
	.thumb
	.thumb_func
	.type	_open, %function
_open:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r3, r4, lr}
	movs	r4, #0
	mov	r3, r2
	strd	r4, r4, [sp, #4]
	mov	r2, r1
	str	r4, [sp]
	mov	r1, r0
	movs	r0, #5
	bl	__system_call(PLT)
	add	sp, sp, #16
	@ sp needed
	pop	{r4, pc}
	.size	_open, .-_open
	.align	1
	.global	_write
	.syntax unified
	.thumb
	.thumb_func
	.type	_write, %function
_write:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r3, r4, lr}
	movs	r4, #0
	mov	r3, r2
	strd	r4, r4, [sp, #4]
	mov	r2, r1
	str	r4, [sp]
	mov	r1, r0
	movs	r0, #4
	bl	__system_call(PLT)
	add	sp, sp, #16
	@ sp needed
	pop	{r4, pc}
	.size	_write, .-_write
	.align	1
	.global	_read
	.syntax unified
	.thumb
	.thumb_func
	.type	_read, %function
_read:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r3, r4, lr}
	movs	r4, #0
	mov	r3, r2
	strd	r4, r4, [sp, #4]
	mov	r2, r1
	str	r4, [sp]
	mov	r1, r0
	movs	r0, #3
	bl	__system_call(PLT)
	add	sp, sp, #16
	@ sp needed
	pop	{r4, pc}
	.size	_read, .-_read
	.align	1
	.global	_close
	.syntax unified
	.thumb
	.thumb_func
	.type	_close, %function
_close:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r3, r4, lr}
	movs	r3, #0
	mov	r1, r0
	strd	r3, r3, [sp, #4]
	str	r3, [sp]
	mov	r2, r3
	movs	r0, #6
	bl	__system_call(PLT)
	add	sp, sp, #20
	@ sp needed
	ldr	pc, [sp], #4
	.size	_close, .-_close
	.align	1
	.global	_getdents
	.syntax unified
	.thumb
	.thumb_func
	.type	_getdents, %function
_getdents:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r3, r4, lr}
	movs	r4, #0
	mov	r3, r2
	strd	r4, r4, [sp, #4]
	mov	r2, r1
	str	r4, [sp]
	mov	r1, r0
	movs	r0, #78
	bl	__system_call(PLT)
	add	sp, sp, #16
	@ sp needed
	pop	{r4, pc}
	.size	_getdents, .-_getdents
	.align	1
	.global	_exit
	.syntax unified
	.thumb
	.thumb_func
	.type	_exit, %function
_exit:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r3, r4, lr}
	movs	r3, #0
	strd	r3, r3, [sp, #4]
	str	r3, [sp]
	mov	r2, r3
	mov	r1, r3
	movs	r0, #1
	bl	__system_call(PLT)
	add	sp, sp, #20
	@ sp needed
	ldr	pc, [sp], #4
	.size	_exit, .-_exit
	.align	1
	.global	_sbrk
	.syntax unified
	.thumb
	.thumb_func
	.type	_sbrk, %function
_sbrk:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	movs	r0, #0
	bx	lr
	.size	_sbrk, .-_sbrk
	.align	1
	.global	_lseek
	.syntax unified
	.thumb
	.thumb_func
	.type	_lseek, %function
_lseek:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	mov	r0, #-1
	bx	lr
	.size	_lseek, .-_lseek
	.align	1
	.global	nanosleep
	.syntax unified
	.thumb
	.thumb_func
	.type	nanosleep, %function
nanosleep:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r3, r4, lr}
	movs	r3, #0
	mov	r1, r0
	strd	r3, r3, [sp, #4]
	str	r3, [sp]
	mov	r2, r3
	movs	r0, #162
	bl	__system_call(PLT)
	add	sp, sp, #20
	@ sp needed
	ldr	pc, [sp], #4
	.size	nanosleep, .-nanosleep
	.align	1
	.global	_execve
	.syntax unified
	.thumb
	.thumb_func
	.type	_execve, %function
_execve:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r3, r4, lr}
	movs	r4, #0
	mov	r3, r2
	strd	r4, r4, [sp, #4]
	mov	r2, r1
	str	r4, [sp]
	mov	r1, r0
	movs	r0, #11
	bl	__system_call(PLT)
	add	sp, sp, #16
	@ sp needed
	pop	{r4, pc}
	.size	_execve, .-_execve
	.ident	"GCC: (15:13.2.rel1-2) 13.2.1 20231009"
