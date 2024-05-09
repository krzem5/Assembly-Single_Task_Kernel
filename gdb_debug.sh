#!/bin/bash
gdb -q -ex "set confirm off" -ex "set pagination off" -ex "target remote localhost:9000" -ex "source src/_build/gdb.py"
