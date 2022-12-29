# Task-scheduler in user space (Linux only)

Complete and functional multicore (M:N) task-scheduler implementation in less than 300 lines of code, including the ASM parts.
Currently, it's only supported for x86_64 with system-V ABI. Supports automatic stack expansion.

Some people call these tasks fibers, user threads, went routines, lightweight threads, and so on... All synonyms for cooperative multitasking without involving HW support. As you can conclude, this is the oldest method of multitasking, which existed in single-core systems, without virtual memory support, decades before OS threads.

This is not supposed to be used as a black box in your project, but as a guideline on how to implement a task scheduler for your particular purposes. In other words, you need a full understanding of this code in order to use it productively.

It is optimized for a large number of tasks and dynamically added tasks by other tasks. If the number of tasks is small, probably os threads would be a better solution.

## Ubuntu install

```
sudo apt install gcc nasm
make
```
Now you can run `example` (uses all cores) and `example1` (uses single core).
