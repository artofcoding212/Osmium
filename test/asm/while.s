; Below is an example of implementation for the while loop while(0){brk;} in Osmium.
; Note - I have not cleaned up the stack after each label, the generated Assmembly from the
; code generator will.

global _start
_start:
    ; start loop
    jmp while;

    while:
        ; while expr
        mov rax, 0
        push rax

        ; check expr == 0
        pop rax
        test rax, rax
        jz end
        ; expr != 0, break
        jmp end

        ; repeat loop
        jmp while

    end:
        ; exit w/ 0
        mov rax, 60
        mov rdi, 0
        syscall