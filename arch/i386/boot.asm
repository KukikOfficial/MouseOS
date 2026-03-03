; Устанавливаем 32-битный режим (Protected Mode)
bits 32

; --- MULTIBOOT HEADER ---
section .text
    align 4
    dd 0x1BADB002              ; magic number
    dd 0x00000007              ; flags: mem info + boot device + VIDEO MODE
    dd - (0x1BADB002 + 0x00000007) ; checksum
    
    ; Поля для VBE (VESA BIOS Extensions)
    dd 0, 0, 0, 0, 0           ; зарезервировано
    dd 0                       ; 0 = Linear Graphics Mode
    dd 1024                    ; Ширина экрана
    dd 768                     ; Высота экрана
    dd 32                      ; Глубина цвета (32 бита)

; --- ОБЪЯВЛЕНИЯ ---
global start
global load_idt
global keyboard_handler_asm
global mouse_handler_asm
global irq_stub

extern kmain
extern keyboard_handler_main
extern mouse_handler_main
extern idt

; --- ТОЧКА ВХОДА ---
start:
    cli                        ; Отключаем прерывания на время настройки
    
    ; 1. Настройка GDT (Global Descriptor Table)
    ; Это фундамент, без которого прерывания вызовут краш
    lgdt [gdt_ptr]
    jmp 0x08:.reload_cs        ; Дальний прыжок для обновления сегмента кода
.reload_cs:
    mov ax, 0x10               ; Сегмент данных (0x10 в нашей GDT)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; 2. Установка стека
    mov esp, stack_top
    
    ; 3. Передача аргументов Multiboot в kmain
    push ebx                   ; адрес структуры Multiboot Info
    push eax                   ; магическое число
    
    call kmain                 ; Прыжок в C++ ядро Mouse Core
    
    ; Если ядро выйдет из kmain - останавливаем процессор
.loop:
    hlt
    jmp .loop

; --- ФУНКЦИИ УПРАВЛЕНИЯ ПРОЦЕССОРОМ ---

; Загрузка IDT
load_idt:
    lidt [idt_ptr_asm]         ; Загружаем таблицу прерываний
    ret

; --- ОБРАБОТЧИКИ ПРЕРЫВАНИЙ (ISR) ---

; Клавиатура (IRQ1)
keyboard_handler_asm:
    pushad                     ; Сохраняем все регистры
    cld                        ; Сброс флага направления для C++
    call keyboard_handler_main
    popad                      ; Восстанавливаем регистры
    iretd                      ; Возврат из прерывания

; Мышь (IRQ12)
mouse_handler_asm:
    pushad
    cld
    call mouse_handler_main
    popad
    iretd

; Универсальная заглушка (EOI для всех IRQ)
irq_stub:
    push eax
    mov al, 0x20
    out 0x20, al               ; Сигнал EOI в Master PIC
    out 0xA0, al               ; Сигнал EOI в Slave PIC
    pop eax
    iretd

; --- ДАННЫЕ (GDT и IDT PTR) ---
section .data
    align 16

; Таблица GDT
gdt_start:
    dq 0x0                     ; Нулевой дескриптор
gdt_code:                      ; Сегмент кода (0x08): base=0, limit=4G, type=code
    dw 0xffff, 0x0000, 0x9a00, 0x00cf
gdt_data:                      ; Сегмент данных (0x10): base=0, limit=4G, type=data
    dw 0xffff, 0x0000, 0x9200, 0x00cf
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Указатель для IDT (формируется в ASM для надежности)
idt_ptr_asm:
    dw (256 * 8) - 1
    dd idt

; --- СТЕК ---
section .bss
    align 16
stack_bottom:
    resb 16384                 ; 16 КБ стека
stack_top:
