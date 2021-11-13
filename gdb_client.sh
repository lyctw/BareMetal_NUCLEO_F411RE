#!/bin/bash
source ./env.sh

# OpenOCD commands: https://openocd.org/doc/html/General-Commands.html
/bin/arm-none-eabi-gdb  -ex "target remote :3333" \
                        -ex "set confirm off" \
                        -ex "set pagination off" \
                        -ex "monitor reset init" \
                        -ex "monitor flash write_image erase final.elf" \
                        -ex "monitor reset halt" \
                        -ex "add-symbol-file final.elf" \
                        -ex "hb main" \
                        -ex "c" final.elf
