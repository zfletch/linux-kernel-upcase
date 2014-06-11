Linux Upcase Kernel Module
==========================

Kernel module based on the tutorial
[Be a Kernel Hacker: Write Your First Linux Kernel Module](http://www.linuxvoice.com/be-a-kernel-hacker)
written by Valentine Sinitsyn.

This module creates a device called `/dev/upcase` which takes a string and converts
any lower case ASCII characters into upper case characters.

How to Compile
==============

- `git clone https://github.com/zfletch/linux-kernel-upcase`
- `cd linux-kernel-upcase/src`
- `make`

How to Insert the Module
=========================

- `sudo insmod upcase.ko`
- `dmesg | tail -n 1` should say `upcase device registered with buffer size 8192 bytes`

How to Test
===========

- `cd ../test`
- `gcc test.c`
- `sudo ./a.out hello` should print `HELLO` (note the sudo)

How to Reload the Module
========================

- `sudo rmmod upcase`
- Make some changes
- `sudo insmod upcase.ko`
