.text

.globl swap_ctx
swap_ctx:
    ldr x1, [x0,0]
    str x19, [x0,0]
    mov x19, x1

    ldr x1, [x0,8]
    str x20, [x0,8]
    mov x20, x1

    ldr x1, [x0,16]
    str x21, [x0,16]
    mov x21, x1

    ldr x1, [x0,24]
    str x22, [x0,24]
    mov x22, x1

    ldr x1, [x0,32]
    str x23, [x0,32]
    mov x23, x1

    ldr x1, [x0,40]
    str x24, [x0,40]
    mov x24, x1

    ldr x1, [x0,48]
    str x25, [x0,48]
    mov x25, x1

    ldr x1, [x0,56]
    str x26, [x0,56]
    mov x26, x1

    ldr x1, [x0,64]
    str x27, [x0,64]
    mov x27, x1

    ldr x1, [x0,72]
    str x28, [x0,72]
    mov x28, x1

    ldr x1, [x0,80]
    str x29, [x0,80]
    mov x29, x1

    ldr x1, [x0,88]
    str x30, [x0,88]
    mov x30, x1

    ldr x1, [x0,96]
    mov x2, sp
    str x2, [x0,96]
    mov sp, x1
    ret

.globl restore_and_unlock
restore_and_unlock:
    ldr x19, [x0,0]
    ldr x20, [x0,8]
    ldr x21, [x0,16]
    ldr x22, [x0,24]
    ldr x23, [x0,32]
    ldr x24, [x0,40]
    ldr x25, [x0,48]
    ldr x26, [x0,56]
    ldr x27, [x0,64]
    ldr x28, [x0,72]
    ldr x29, [x0,80]
    ldr x30, [x0,88]
    ldr x2, [x0,96]
    mov sp, x2
    mov x2, 0
    str x2, [x1,0]
    ret

.globl init_ctx
init_ctx:
    and x0, x0, 0xfffffffffffffff0
    sub x0, x0, 128
    mov x5, 0

    str x5, [x0,0]
    str x5, [x0,8]
    str x5, [x0,16]
    str x5, [x0,24]
    str x5, [x0,32]
    str x5, [x0,40]
    str x5, [x0,48]
    str x5, [x0,56]
    str x5, [x0,64]
    str x5, [x0,72]
    str x5, [x0,80] /* FP */
    sub x5, x0, 16
    str x5, [x0,96] /* SP */

    str x4, [x5,-8]
    str x3, [x5,-16]
    str x2, [x5,-24]
    str x1, [x5,-32]

    adr x5, task_start
    str x5, [x0,88] /* RET */

    ret

task_start:
    ldr x2, [sp,-8]
    ldr x1, [sp,-16]
    ldr x0, [sp,-24]
    ldr x3, [sp,-32]
    br x3
