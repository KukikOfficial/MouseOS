#include "include/keyboard.h"
#include "include/vga.h"
#include "include/io.h"
#include "include/fs.h"
#include "include/ata.h"

// Глобальные переменные консоли
extern bool shift_held;
extern char command_buffer[128];
extern int cmd_ptr;

// Импортируем состояния Блокнота
extern bool notepad_open;
extern char notepad_buffer[256];
extern int notepad_ptr;

// ИСПРАВЛЕНО: Теперь объявление функции находится на глобальном уровне, где ему и место
extern "C" void draw_all_ui();

// Карты символов
unsigned char kbd_map[] = { 0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' ' };
unsigned char kbd_map_shift[] = { 0, 27, '!','@','#','$','%','^','&','*','(',')','_','+','\b','\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,'A','S','D','F','G','H','J','K','L',':','\"','~',0,'|','Z','X','C','V','B','N','M','<','>','?',0,'*',0,' ' };

bool str_compare(const char* s1, const char* s2) {
    int i = 0;
    while(s1[i] && s2[i]) { if(s1[i] != s2[i]) return false; i++; }
    return s1[i] == s2[i];
}

bool str_starts(const char* str, const char* prefix) {
    int i = 0;
    while(prefix[i]) { if(str[i] != prefix[i]) return false; i++; }
    return true;
}

void execute_command() {
    // Твоя логика обработки команд для VGA режима...
}

extern "C" void keyboard_handler_main() {
    uint8_t scancode = inb(0x60);

    if (scancode == 0x2A || scancode == 0x36) shift_held = true;
    else if (scancode == 0xAA || scancode == 0xB6) shift_held = false;
    else if (!(scancode & 0x80)) {
        char c = shift_held ? kbd_map_shift[scancode] : kbd_map[scancode];
        
        // Если открыт Блокнот, перехватываем ввод прямо в графический буфер
        if (notepad_open) {
            if (c == '\b' && notepad_ptr > 0) { // Нажат Backspace
                notepad_ptr--;
                notepad_buffer[notepad_ptr] = '\0';
                draw_all_ui(); // Перерисовываем UI, чтобы увидеть изменения
            } 
            else if (c >= ' ' && notepad_ptr < 250) { // Обычный символ
                notepad_buffer[notepad_ptr++] = c;
                notepad_buffer[notepad_ptr] = '\0';
                draw_all_ui(); 
            }
        } 
        else {
            // Если блокнот закрыт, ввод идет в старый буфер консоли
            if (c == '\n') execute_command();
            else if (c == '\b' && cmd_ptr > 0) {
                cmd_ptr--;
                terminal_putchar('\b');
            } else if (c >= ' ' && cmd_ptr < 127) {
                command_buffer[cmd_ptr++] = c;
                terminal_putchar(c);
            }
        }
    }
    outb(0x20, 0x20);
}