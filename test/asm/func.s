; Below is an example of implementation for the function defintion in Osmium, it follows this Osmium code:
; func add(x: int, y: int){ ret x + y; } func main(){ ret add(2, 2); }

add:
push rbp
mov rbp, rsp
push QWORD [rbp + 16]
push QWORD [rbp + 24]
pop rax
pop rbx
add rax, rbx
push rax
pop rax
mov rsp, rbp
pop rbp
ret
global _start
_start:
mov rax, 2
push rax
mov rax, 2
push rax
call add
push rax
mov rax, 60
pop rdi
syscall
