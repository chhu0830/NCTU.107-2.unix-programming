
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

    gensys  1, write
    gensys 34, pause
    gensys 37, alarm
    gensys 60, exit
