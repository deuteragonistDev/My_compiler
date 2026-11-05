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

;encerra programa
mov ebx,0
mov eax,1
int 0x80

	section .data
x: dd "%d", 4
z: dd "%lf", 16
nome: db "                " 
letra: dd "%c", 1
str0: db "Maria"
str1: db 'A'

	section .data
x: dd "%d", 4
z: dd "%lf", 16
nome: db "                " 
letra: dd "%c", 1
str0: db "Maria"
str1: db 'A'
