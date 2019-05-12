
%macro gensys 2
    global sys_%2:function
sys_%2:
    push r10
    mov r10, rcx
    mov rax, %1
    syscall
    pop r10
    ret
%endmacro

extern errno

    section .data

    section .text

    gensys   1, write
    gensys  13, rt_sigaction
    gensys  14, rt_sigprocmask
    gensys  34, pause
    gensys  35, nanosleep
    gensys  37, alarm
    gensys  60, exit
    gensys 127, rt_sigpending


    global sigrestore:function
sigrestore:
    mov rax, 15
    syscall
    ret


    global setjmp:function
setjmp:
    mov [rdi], rbx
    mov [rdi+0x08], rbp
    mov [rdi+0x10], rsp
    mov [rdi+0x18], r12
    mov [rdi+0x20], r13
    mov [rdi+0x28], r14
    mov [rdi+0x30], r15
    mov rax, [rsp]              ; rip
    mov [rdi+0x38], rax

    mov rdi, 0
    mov rsi, 0
    lea rdx, [rdi+0x40]
    call sys_rt_sigprocmask     ; sigprocmask(0, NULL, &mask);

    mov rax, 0
    ret


    global longjmp:function
longjmp:
    mov rbx, [rdi]
    mov rbp, [rdi+0x08]
    mov rsp, [rdi+0x10]
    mov r12, [rdi+0x18]
    mov r13, [rdi+0x20]
    mov r14, [rdi+0x28]
    mov r15, [rdi+0x30]
    
    pop rax
    mov rax, [rdi+0x38]
    push rax

    push rsi
    mov rdi, 2
    lea rsi, [rdi+0x40]
    mov rdx, 0
    call sys_rt_sigprocmask     ; sigprocmask(SIG_SETMASK, &mask, 0);
    pop rsi

    cmp rsi, 0
    jne next
    mov rsi, 1
next:
    mov rax, rsi
    ret
