#!/bin/bash
gdb -ex "set confirm off" -ex "set pagination off" -ex "add-symbol-file build/kernel/kernel.elf" -ex "target remote localhost:9000"
