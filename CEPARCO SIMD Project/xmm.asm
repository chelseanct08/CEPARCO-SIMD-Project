; AVX packed-double dot product using 128-bit XMM registers.
; Windows x64: RCX = A, RDX = B, R8 = n, XMM0 = return value.

.code

PUBLIC dot_xmm

dot_xmm PROC
    xor     eax, eax
    vxorpd  xmm0, xmm0, xmm0
    mov     r9, r8
    shr     r9, 1
    test    r9, r9
    jz      dot_xmm_reduce

dot_xmm_loop:
    vmovupd xmm1, xmmword ptr [rcx + rax*8]
    vmovupd xmm2, xmmword ptr [rdx + rax*8]
    vmulpd  xmm1, xmm1, xmm2
    vaddpd  xmm0, xmm0, xmm1
    add     rax, 2
    dec     r9
    jnz     dot_xmm_loop

dot_xmm_reduce:
    vhaddpd xmm0, xmm0, xmm0
    test    r8, 1
    jz      dot_xmm_done

    vmovsd  xmm1, qword ptr [rcx + rax*8]
    vmulsd  xmm1, xmm1, qword ptr [rdx + rax*8]
    vaddsd  xmm0, xmm0, xmm1

dot_xmm_done:
    vzeroupper
    ret
dot_xmm ENDP

END