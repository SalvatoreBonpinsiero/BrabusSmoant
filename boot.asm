bits 32

MBOOT_PAGE_ALIGN    equ 1 << 0
MBOOT_MEM_INFO      equ 1 << 1
MBOOT_VID_MODE      equ 1 << 2
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_VID_MODE
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

section .multiboot
    align 4
    dd MBOOT_HEADER_MAGIC
    dd MBOOT_HEADER_FLAGS
    dd MBOOT_CHECKSUM
    dd 0, 0, 0, 0, 0
    dd 0            ; 0 = Linear Graphics
    dd 1024         ; Width
    dd 768          ; Height
    dd 32           ; 32 bpp TrueColor

section .text
global start
extern kernel_main

start:
    cli
    mov esp, stack_space
    push ebx        ; Передаем указатель на multiboot_info
    call kernel_main
    hlt

section .bss
align 16
resb 65536
stack_space:
