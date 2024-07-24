; Below is an example of implementation for the if statement if(0){}elif(0){}else{} in Osmium.
; Note - I have not cleaned up the stack after each label, the generated Assmembly from the
; code generator will.

global _start
_start:
    ; if expr
    mov rax, 0
    push rax

    ; check expr == 0
    pop rax
    test rax, rax
    jz if_unsuccess
    ; expr != 0, goto end
    jmp end;

    if_unsuccess:
        ; elif expr
        mov rax, 0
        push rax

        ; check expr == 0
        pop rax
        test rax, rax
        jz elif_unsuccess
        ; expr != 0, goto end
        jmp end;

        elif_unsuccess:
            ; else
            jmp end;

    end:
        ; exit w/ 0
        mov rax, 60
        mov rdi, 0
        syscall;