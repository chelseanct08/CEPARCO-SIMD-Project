; AVX2 packed-double dot product using 256-bit YMM registers.
; Windows x64: RCX = A, RDX = B, R8 = n, XMM0 = return value.

.code

PUBLIC dot_ymm

dot_ymm PROC
    xor     eax, eax
    vxorpd  ymm0, ymm0, ymm0
    mov     r9, r8
    shr     r9, 2
    test    r9, r9
    jz      dot_ymm_reduce

dot_ymm_loop:
    vmovupd ymm1, ymmword ptr [rcx + rax*8]
    vmovupd ymm2, ymmword ptr [rdx + rax*8]
    vmulpd  ymm1, ymm1, ymm2
    vaddpd  ymm0, ymm0, ymm1
    add     rax, 4
    dec     r9
    jnz     dot_ymm_loop

dot_ymm_reduce:
    vextracti128 xmm1, ymm0, 1
    vaddpd      xmm0, xmm0, xmm1
    vhaddpd     xmm0, xmm0, xmm0

dot_ymm_tail:
    cmp     rax, r8
    jae     dot_ymm_done
    vmovsd  xmm1, qword ptr [rcx + rax*8]
    vmulsd  xmm1, xmm1, qword ptr [rdx + rax*8]
    vaddsd  xmm0, xmm0, xmm1
    inc     rax
    jmp     dot_ymm_tail

dot_ymm_done:
    vzeroupper
    ret
dot_ymm ENDP

END