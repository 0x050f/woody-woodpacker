# woody-woodpacker

The goal of this project is to code a program that will encrypt a elf 64 bits binary given as parameter..A new program called `woody` will be generated from this execution. When this new program (woody) will be executed, it will print "....WOODY...." then decrypt itself (the .text section of `woody`) before running.

### Disclaimer:

Only works on linux elf 64 bits binaries.

## Compilation

```
make
```

## Execution

```
usage: ./woody_woodpacker file
```

## Demo

![alt text](https://github.com/ska42/woody-woodpacker/blob/main/img/demo.png?raw=true)

## How does it works ?

![alt text](https://github.com/ska42/woody-woodpacker/blob/main/img/scheme.png?raw=true)

We will add the injection after all sections in the first PT_LOAD executable segment,
if the remaining space between the two PT_LOAD segments is not enough, we will just add
padding (4096 bytes on linux 64 bits) on all sections offset and segments offset after it.

The `woody-woodpacker` program will encrypt .text section using XOR algorithm of a key of 
n bytes. Moving each bytes from key after each bytes encrypted. The injection would have to
decrypt the .text section before moving to it. (using mprotect to be able to edit the mmaped region).
