#!/bin/bash
source ./env.sh

# Cannot use over 6 breakpoints
# OpenOCD commands: https://openocd.org/doc/html/General-Commands.html
/bin/arm-none-eabi-gdb  -ex "target remote :3333" \
                        -ex "set confirm off" \
                        -ex "set pagination off" \
                        -ex "monitor reset init" \
                        -ex "monitor flash write_image erase final.elf" \
                        -ex "monitor reset halt" \
                        -ex "add-symbol-file final.elf" \
                        -ex "b idle_task" \
                        -ex "b task1_handler" \
                        -ex "b task3_handler" \
                        -ex "b task4_handler" \
                        -ex "b SysTick_Handler" \
                        -ex "b PendSV_Handler" \
                        -ex "c" final.elf
