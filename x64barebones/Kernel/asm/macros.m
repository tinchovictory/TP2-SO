
%macro pushaq 0
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push fs
	push gs
%endmacro

%macro popaq 0
	pop gs
	pop fs
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

EXTERN acaEstoy

%macro irqHandler 1
	pushaq

	mov rdi, %1
	call irqDispatcher

	;call acaEstoy
	
	mov al, 20h ; EOI
	out 20h, al

	;call acaEstoy
	
	popaq

	iretq
%endmacro

%macro irqHandlerSlave 1
	pushaq

	mov rdi, %1
	call irqDispatcher
	
	mov al, 20h ; EOI
	out 0A0h, al ; moving EOI to PIC2_COMMAND

	mov al, 20h ; EOI
	out 20h, al ; moving EOI to PIC1_COMMAND
	
	popaq

	iretq
%endmacro

