.code

PUBLIC dot_x64

dot_x64 PROC
    xor     eax, eax
    xorpd   xmm0, xmm0

dot_x64_loop:
    cmp     rax, r8
    jae     dot_x64_done

    movsd   xmm1, qword ptr [rcx + rax*8]
    movsd   xmm2, qword ptr [rdx + rax*8]

    mulsd   xmm1, xmm2
    addsd   xmm0, xmm1

    inc     rax
    jmp     dot_x64_loop

dot_x64_done:
    ret
dot_x64 ENDP

END