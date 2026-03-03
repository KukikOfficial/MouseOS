bits 32

section .text
    align 4
    dd 0x1BADB002              ; magic
    dd 0x00000007              ; flags: mem info + boot device + VIDEO MODE
    dd - (0x1BADB002 + 0x00000007)
    
    ; Добавляем параметры для VESA (пустые поля для заполнения загрузчиком)
    dd 0, 0, 0, 0, 0
    dd 0                       ; 0 = Linear Graphics Mode
    dd 1024                    ; Ширина
    dd 768                     ; Высота
    dd 32                      ; Глубина цвета (32 бита = RGBA)

global start
global load_idt
global keyboard_handler_asm
extern kmain
extern keyboard_handler_main

start:
    cli
    lgdt [gdt_ptr]         ; 1. Загружаем НАШУ таблицу сегментов
    jmp 0x08:.reload_cs    ; 2. Дальний прыжок для обновления сегмента кода
.reload_cs:
    mov ax, 0x10           ; 0x10 - это сегмент данных в нашей GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov esp, stack_top
    call kmain
    hlt

load_idt:
    lidt [idt_ptr_asm]
    ret

keyboard_handler_asm:
    pushad
    cld
    call keyboard_handler_main
    popad
    iretd

section .data
    align 16
; --- НАША GDT ---
gdt_start:
    dq 0x0                 ; Нулевой дескриптор
gdt_code:                  ; Сегмент кода: base=0, limit=ffff, type=9a, flags=cf (0x08)
    dw 0xffff, 0x0000, 0x9a00, 0x00cf
gdt_data:                  ; Сегмент данных: base=0, limit=ffff, type=92, flags=cf (0x10)
    dw 0xffff, 0x0000, 0x9200, 0x00cf
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; --- НАША IDT ---
global idt
idt:
    times 256 dq 0

idt_ptr_asm:
    dw (256 * 8) - 1
    dd idt

section .bss
    align 16
stack_bottom:
    resb 16384
stack_top:
