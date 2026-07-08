#include "include/keyboard.h"
#include "include/vga.h"
#include "include/io.h"
#include "include/fs.h"
#include "include/ata.h"

bool shift_held = false;
char command_buffer[128];
int cmd_ptr = 0;

unsigned char kbd_map[] = { 0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' ' };
unsigned char kbd_map_shift[] = { 0, 27, '!','@','#','$','%','^','&','*','(',')','_','+','\b','\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,'A','S','D','F','G','H','J','K','L',':','\"','~',0,'|','Z','X','C','V','B','N','M','<','>','?',0,'*',0,' ' };

extern "C" {

extern bool notepad_open;
extern char notepad_buffer[256];
extern int notepad_ptr;
void draw_all_ui();

bool kbd_str_compare(const char* s1, const char* s2) {
    int i = 0;
    while(s1[i] && s2[i]) { if(s1[i] != s2[i]) return false; i++; }
    return s1[i] == s2[i];
}

bool kbd_str_starts(const char* str, const char* prefix) {
    int i = 0;
    while(prefix[i]) { if(str[i] != prefix[i]) return false; i++; }
    return true;
}

void execute_command() {
    terminal_putchar('\n');
    command_buffer[cmd_ptr] = '\0';

    if (kbd_str_compare(command_buffer, "help")) {
        terminal_writestring("Commands: ls, cat <file>, echo \"text\" > file, clear, reboot");
    } 
    else if (kbd_str_compare(command_buffer, "ls")) {
        list_files();
    }
    else if (kbd_str_starts(command_buffer, "cat ")) {
        char* filename = &command_buffer[4];
        while (*filename == ' ') filename++;
        read_from_disk(filename);
    }
    else if (kbd_str_starts(command_buffer, "echo \"")) {
        char *text_start = 0, *text_end = 0, *filename = 0;
        
        for (int i = 0; i < cmd_ptr; i++) {
            if (command_buffer[i] == '\"' && !text_start) text_start = &command_buffer[i+1];
            else if (command_buffer[i] == '\"' && text_start) text_end = &command_buffer[i];
            else if (command_buffer[i] == '>') filename = &command_buffer[i+1];
        }

        if (text_start && text_end && filename) {
            *text_end = '\0'; 
            while (*filename == ' ') filename++; 
            write_to_disk(filename, text_start);
        } else {
            terminal_writestring("Usage: echo \"text\" > file");
        }
    }
    else if (kbd_str_compare(command_buffer, "clear")) {
        terminal_clear();
    }
    else if (kbd_str_compare(command_buffer, "reboot")) {
        sys_reboot();
    }
    else if (cmd_ptr > 0) {
        terminal_writestring("MouseCore: Command not found.");
    }

    terminal_writestring("\nMouseCore# ");
    cmd_ptr = 0;
}

void keyboard_handler_main() {
    uint8_t scancode = inb(0x60);

    if (scancode == 0x2A || scancode == 0x36) shift_held = true;
    else if (scancode == 0xAA || scancode == 0xB6) shift_held = false;
    else if (!(scancode & 0x80)) {
        char c = shift_held ? kbd_map_shift[scancode] : kbd_map[scancode];
        
        if (notepad_open) {
            if (c == '\b' && notepad_ptr > 0) { 
                notepad_ptr--;
                notepad_buffer[notepad_ptr] = '\0';
                draw_all_ui(); 
            } 
            else if (c >= ' ' && notepad_ptr < 250) { 
                notepad_buffer[notepad_ptr++] = c;
                notepad_buffer[notepad_ptr] = '\0';
                draw_all_ui(); 
            }
        } 
        else {
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

} // extern "C"