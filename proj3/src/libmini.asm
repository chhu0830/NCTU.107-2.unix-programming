
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
