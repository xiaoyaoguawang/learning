.code
EnableVmx PROC PUBLIC
	; Enable VMX operation by setting the VMXE bit in CR4
	push rax
	mov rax,cr4
	or rax,2000h
	mov cr4,rax
	pop rax
	ret
EnableVmx ENDP
END