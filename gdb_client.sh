#!/bin/bash

source ./env.sh

# OpenOCD commands: https://openocd.org/doc/html/General-Commands.html
arm-none-eabi-gdb -ex "target remote :3333" \
                  -ex "monitor reset init" \
                  -ex "monitor flash write_image erase final.elf" \
                  -ex "monitor reset halt"
