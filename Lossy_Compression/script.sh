#!/bin/bash

for ((i=1; i <= 8; i = i * 2)) do
	./ppu/lab8_ppu 0 0 $i ./../tema3_input/in1.pgm myout1.cmp myout1.pgm mod_vect0.out
	./ppu/lab8_ppu 1 0 $i ./../tema3_input/in2.pgm myout2.cmp myout2.pgm mod_vect1.out
	./ppu/lab8_ppu 2 0 $i ./../tema3_input/in3.pgm myout3.cmp myout3.pgm mod_vect2.out
done

for ((i=1; i <= 8; i = i * 2)) do
	./ppu/lab8_ppu 0 0 $i ./../tema3_input/in4.pgm myout4.cmp myout4.pgm compare0.out
	./ppu/lab8_ppu 1 0 $i ./../tema3_input/in4.pgm myout4.cmp myout4.pgm compare1.out
	./ppu/lab8_ppu 2 0 $i ./../tema3_input/in4.pgm myout4.cmp myout4.pgm compare2.out

	./ppu/lab8_ppu 1 1 $i ./../tema3_input/in4.pgm myout4.cmp myout4.pgm compare1dma1.out
done

