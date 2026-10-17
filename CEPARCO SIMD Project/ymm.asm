.code

PUBLIC dot_ymm

dot_ymm PROC
    xor     eax, eax
    vxorpd  ymm0, ymm0, ymm0

    mov     r9, r8
    and     r9, -4

dot_ymm_loop:
    cmp     rax, r9
    jae     dot_ymm_reduce

    vmovupd ymm1, ymmword ptr [rcx + rax*8]
    vmovupd ymm2, ymmword ptr [rdx + rax*8]

    vmulpd  ymm1, ymm1, ymm2
    vaddpd  ymm0, ymm0, ymm1

    add     rax, 4
    jmp     dot_ymm_loop

dot_ymm_reduce:
    vextractf128 xmm1, ymm0, 1
    vaddpd      xmm0, xmm0, xmm1
    vhaddpd     xmm0, xmm0, xmm0

    mov     r10, r9

dot_ymm_tail:
    cmp     r10, r8
    jae     dot_ymm_done

    movsd   xmm1, qword ptr [rcx + r10*8]
    movsd   xmm2, qword ptr [rdx + r10*8]

    mulsd   xmm1, xmm2
    addsd   xmm0, xmm1

    inc     r10
    jmp     dot_ymm_tail

dot_ymm_done:
    vzeroupper
    ret
dot_ymm ENDP

END