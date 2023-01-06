section .text
    global init_ctx
    global swap_ctx
    global restore_and_unlock

swap_ctx:
    pop rax
    mov r9, [rdi+0]
    mov [rdi+0], rax

    mov rax, [rdi+8]
    mov [rdi+8], rsp
    mov rsp, rax

    mov rax, [rdi+16]
    mov [rdi+16], rbp
    mov rbp, rax

    mov rax, [rdi+24]
    mov [rdi+24], rbx
    mov rbx, rax

    mov rax, [rdi+32]
    mov [rdi+32], r12
    mov r12, rax

    mov rax, [rdi+40]
    mov [rdi+40], r13
    mov r13, rax

    mov rax, [rdi+48]
    mov [rdi+48], r14
    mov r14, rax

    mov rax, [rdi+56]
    mov [rdi+56], r15
    mov r15, rax

    jmp r9

init_ctx:
    mov rax, rdi
    and rax, 0xfffffffffffffff0
    sub rax, 72

    mov qword[rax+56], 0
    mov qword[rax+48], 0
    mov qword[rax+40], 0
    mov qword[rax+32], 0
    mov qword[rax+24], 0

    mov [rax+16], rax ; BP
    mov [rax+8], rax ; SP

    mov [rax-8], r8 ; arg2
    mov [rax-16], rcx ; arg1
    mov [rax-24], rdx ; arg0
    mov [rax-32], rsi ; entry

    mov r9, task_start
    mov [rax], r9 ; IP

    ret

restore_and_unlock:
    mov r9, [rdi]
    mov rsp, [rdi+8]
    mov rbp, [rdi+16]
    mov rbx, [rdi+24]
    mov r12, [rdi+32]
    mov r13, [rdi+40]
    mov r14, [rdi+48]
    mov r15, [rdi+56]
    mov dword[rsi], 0

    jmp r9

task_start:
    mov rdx, [rbp-8]
    mov rsi, [rbp-16]
    mov rdi, [rbp-24]
    mov r9, [rbp-32]
    jmp r9
