;UFMT-Compiladores
;Prof. Ivairton
;Procedimento para geracao do executavel apos compilacao (em Linux):
;(1) compilacao do Assembly com nasm: $ nasm -f elf64 <nome_do_arquivo>
;(2) likedicao: $ ld -m elf_x86_64 <nome_arquivo_objeto>

extern printf
extern scanf

	section .text
	global main,_start
main:
_start:
;Amarzenamento de numero
mov rax,1
push rax
pop rax
mov [x],eax
;Amarzenamento de numero
mov rax,2
push rax
pop rax
mov [y],eax
call func_soma

;encerra programa
mov ebx,0
mov eax,1
int 0x80
func_soma:
mov eax,[x]
push rax
mov eax,[y]
push rax
;Adicao
pop rax
pop rbx
add rax,rbx
push rax
pop rax
mov [a],eax
ret

	section .data
a: dd 0
x: dd 0
y: dd 0
