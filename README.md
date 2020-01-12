This project is made to control my 3x3x3 or 3x6x3 RGB led cubes.

The hardware and software are of my own design. The harware includes 2 TLC5940 and an Arduino Nano.
The led cube is multiplexed in layers and data is send to the TLC's using SPI.

This version is optimized to run on minimal memory using only 8 bits instead of the 12 for each color.
Many variables are shared amongst all animations, saving a lot memory.

See my cubes and other projects running here: https://www.youtube.com/channel/UCuQumwFU8Kvs-C-DP2EolAw
