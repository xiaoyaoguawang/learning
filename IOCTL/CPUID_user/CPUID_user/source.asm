option casemap:none
.code 
GetCpuID PROC
	push rcx
	xor eax,eax
	cpuid
	mov eax , ecx
	pop rcx
	mov dword ptr [rcx],ebx
	mov dword ptr [rcx+4],edx
	mov dword ptr [rcx+8],eax
	mov byte ptr [rcx+12],0
	ret
GetCpuID ENDP

DetectVmxSupport PROC
	xor eax,eax
	inc eax
	cpuid
	bt ecx,5
	setc al
	movzx eax,al
	ret
DetectVmxSupport ENDP


END