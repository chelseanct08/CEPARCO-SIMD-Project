.code

PUBLIC dot_xmm

dot_xmm PROC
    xor     eax, eax
    vxorpd  xmm0, xmm0, xmm0

    mov     r9, r8
    and     r9, -2

dot_xmm_loop:
    cmp     rax, r9
    jae     dot_xmm_reduce

    vmovupd xmm1, xmmword ptr [rcx + rax*8]
    vmovupd xmm2, xmmword ptr [rdx + rax*8]

    vmulpd  xmm1, xmm1, xmm2
    vaddpd  xmm0, xmm0, xmm1

    add     rax, 2
    jmp     dot_xmm_loop

dot_xmm_reduce:
    vhaddpd xmm0, xmm0, xmm0

    cmp     r9, r8
    jae     dot_xmm_done

    movsd   xmm1, qword ptr [rcx + r9*8]
    movsd   xmm2, qword ptr [rdx + r9*8]

    mulsd   xmm1, xmm2
    addsd   xmm0, xmm1

dot_xmm_done:
    vzeroupper
    ret
dot_xmm ENDP

END