#!/usr/bin/python

from pylab import plot, show, arange, legend, xlabel, ylabel
from train import run
from data_loader_mnist import load_mnist
from data_loader_cifrar import load_cifrar
from feed_forward import LINEARIZE, FULLY_CONNECTED, TANH, SOFTMAX, CONV, RELU, MAX_POOLING

xlabel('Numarul de imagini de antrenare')
ylabel('Acuratete Teste')

cifar = load_cifrar()
arhitecture1 = [(CONV, (6, 14, 14), 6, 2), (RELU, -1), (MAX_POOLING, (6, 7, 7)), (LINEARIZE, -1), (FULLY_CONNECTED, 49), (FULLY_CONNECTED, 10), (SOFTMAX, -1)]
arhitecture2 = [(CONV, (9, 14, 14), 6, 2), (RELU, -1), (MAX_POOLING, (9, 7, 7)), (LINEARIZE, -1), (FULLY_CONNECTED, 49), (FULLY_CONNECTED, 10), (SOFTMAX, -1)]

index = 0
for i in (arhitecture1, arhitecture2):
	index += 1
	(inputTrainC, outputTrainC, inputTestC, outputTestC) = run(cifar, i, 0.002, 2000, 10000) 
	x = inputTestC
	y = outputTestC
	plot(x, y, label="Architecture = " + str(index))

legend(prop={'size':6})
show()