/* Windows x64 pre-call observer. Keep original stack/return address untouched.
 * Save all volatile GPRs and XMM registers, including the fourth float argument.
 * C observer obeys the ABI for nonvolatile registers. SEH describes our frame.
 * No displaced instructions, guessed return type, or post-call wrapper. */
.text
.globl tfx_terminal_entry
.def tfx_terminal_entry; .scl 2; .type 32; .endef
.seh_proc tfx_terminal_entry
tfx_terminal_entry:
    subq $0xc8, %rsp
    .seh_stackalloc 0xc8
    .seh_endprologue
    movq %rax, 0x20(%rsp)
    movq %rcx, 0x28(%rsp)
    movq %rdx, 0x30(%rsp)
    movq %r8,  0x38(%rsp)
    movq %r9,  0x40(%rsp)
    movq %r10, 0x48(%rsp)
    movq %r11, 0x50(%rsp)
    movaps %xmm0, 0x60(%rsp)
    movaps %xmm1, 0x70(%rsp)
    movaps %xmm2, 0x80(%rsp)
    movaps %xmm3, 0x90(%rsp)
    movaps %xmm4, 0xa0(%rsp)
    movaps %xmm5, 0xb0(%rsp)
    call tfx_observe
    movaps 0x60(%rsp), %xmm0
    movaps 0x70(%rsp), %xmm1
    movaps 0x80(%rsp), %xmm2
    movaps 0x90(%rsp), %xmm3
    movaps 0xa0(%rsp), %xmm4
    movaps 0xb0(%rsp), %xmm5
    movq 0x20(%rsp), %rax
    movq 0x28(%rsp), %rcx
    movq 0x30(%rsp), %rdx
    movq 0x38(%rsp), %r8
    movq 0x40(%rsp), %r9
    movq 0x48(%rsp), %r10
    movq 0x50(%rsp), %r11
    addq $0xc8, %rsp
    jmp *tfx_original(%rip)
.seh_endproc
