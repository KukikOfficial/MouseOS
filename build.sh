#!/bin/bash

# Остановить выполнение скрипта, если любая команда завершится с ошибкой
set -e

# --- Переменные ---
# Устанавливаем имя для кросс-компилятора. Замените, если у вас другой.
export PREFIX="i686-elf-"
export CC="${PREFIX}g++"
export AS="nasm"

# Флаги компилятора C++
# -Ikernel/include: добавляем директорию с нашими заголовочными файлами
# -ffreestanding: мы не используем стандартную библиотеку ОС
# -fno-exceptions, -fno-rtti: отключаем исключения и информацию о типах времени выполнения
# -Wall, -Wextra: включаем все предупреждения
CFLAGS="-std=gnu++17 -Ikernel/include -ffreestanding -nostdinc++ -O2 -Wall -Wextra -fno-exceptions -fno-rtti"

# Флаги компоновщика
# -T: указываем наш скрипт компоновщика
# -nostdlib: не подключаем стандартные библиотеки
# -lgcc: но нам нужна вспомогательная библиотека от gcc
LDFLAGS="-T arch/i386/linker.ld -ffreestanding -O2 -nostdlib -lgcc"

# --- Проверка на наличие кросс-компилятора ---
if ! command -v ${CC} &> /dev/null
then
    echo "Ошибка: Кросс-компилятор ${CC} не найден."
    echo "Пожалуйста, установите i686-elf-toolchain."
    echo "Инструкции можно найти на wiki.osdev.org"
    exit 1
fi

# --- Сборка ---

echo "Создание директорий сборки..."
mkdir -p build/obj build/iso/boot/grub

echo "Сборка загрузчика boot.asm..."
${AS} -f elf32 arch/i386/boot.asm -o build/obj/boot.o

echo "Компиляция ядра C++..."
# Core modules
${CC} ${CFLAGS} -c kernel/kernel.cpp -o build/obj/kernel.o
${CC} ${CFLAGS} -c kernel/vga.cpp -o build/obj/vga.o
# new subsystems
#${CC} ${CFLAGS} -c kernel/usb.cpp -o build/obj/usb.o
#${CC} ${CFLAGS} -c kernel/fs.cpp -o build/obj/fs.o
#${CC} ${CFLAGS} -c kernel/scheduler.cpp -o build/obj/scheduler.o
#${CC} ${CFLAGS} -c kernel/interrupts.cpp -o build/obj/interrupts.o
# assemble scheduler assembly (stack switch, EOI helper)
#as -f elf32 kernel/scheduler.s -o build/obj/scheduler_asm.o
${CC} ${CFLAGS} -c kernel/keyboard.cpp -o build/obj/keyboard.o
${CC} ${CFLAGS} -c kernel/idt.cpp -o build/obj/idt.o 
${CC} ${CFLAGS} -c kernel/fs.cpp -o build/obj/fs.o
${CC} ${CFLAGS} -c kernel/ata.cpp -o build/obj/ata.o
${CC} ${CFLAGS} -c kernel/memory.cpp -o build/obj/memory.o
${CC} ${CFLAGS} -c kernel/vesa.cpp -o build/obj/vesa.o
${CC} ${CFLAGS} -c kernel/font.cpp -o build/obj/font.o
${CC} ${CFLAGS} -c kernel/mouse.cpp -o build/obj/mouse.o

echo "Компоновка ядра..."
# Собираем все .o файлы в один бинарный файл ядра
${CC} ${LDFLAGS} -o build/iso/boot/myos.bin build/obj/boot.o build/obj/kernel.o build/obj/vga.o build/obj/keyboard.o build/obj/idt.o build/obj/fs.o build/obj/ata.o build/obj/memory.o build/obj/vesa.o build/obj/font.o build/obj/mouse.o

echo "Создание конфигурации GRUB..."
# add a sample module file that will be loaded by GRUB
mkdir -p build/iso/boot
cat > build/iso/boot/hello.txt << 'EOM'
This file was loaded as a multiboot module!
EOM

cat > build/iso/boot/grub/grub.cfg << EOF
menuentry "myos" {
    multiboot /boot/myos.bin
    module /boot/hello.txt
}
EOF

echo "Создание ISO-образа..."
grub-mkrescue -o build/myos.iso build/iso

echo -e "\n\033[0;32mСборка успешно завершена!\033[0m"
echo "Запустите ваше ядро с помощью команды:"
echo "qemu-system-i386 -cdrom build/myos.iso"