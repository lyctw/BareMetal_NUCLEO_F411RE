# Bare metal on NUCLEO F411RE

## OpenOCD Installation

Follow the instructions here: https://github.com/STMicroelectronics/OpenOCD/tree/master

Allow unprivileged users to run OpenOCD: `sudo vim /etc/udev/rules.d/stlink.rules`

```
KERNEL=="tty[A-Z]*[0-9]", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", MODE="0666"
```


