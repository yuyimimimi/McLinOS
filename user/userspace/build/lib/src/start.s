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
	.file	"start.c"
	.text
	.align	1
	.syntax unified
	.thumb
	.thumb_func
	.type	reset_handler, %function
reset_handler:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	mov	r1, r0
	movs	r0, #0
.L2:
	ldr	r3, [r1, r0, lsl #2]
	cbnz	r3, .L3
	b	main(PLT)
.L3:
	adds	r0, r0, #1
	b	.L2
	.size	reset_handler, .-reset_handler
	.global	head
	.section	.appmeta,"aw"
	.align	2
	.type	head, %object
	.size	head, 36
head:
	.word	12345678
	.word	0
	.word	1
	.word	7
	.word	8192
	.word	reset_handler
	.word	_got_start
	.word	_got_end
	.word	__end_program
	.ident	"GCC: (15:13.2.rel1-2) 13.2.1 20231009"
