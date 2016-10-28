#!/usr/bin/python

from pylab import plot, show, arange, legend, xlabel, ylabel
from train import run
from data_loader_mnist import load_mnist
from data_loader_cifrar import load_cifrar
from feed_forward import LINEARIZE, FULLY_CONNECTED, TANH, SOFTMAX

xlabel('Numarul de imagini de antrenare')
ylabel('Acuratete Teste')

cifar = load_cifrar()
arhitecture1 = [(LINEARIZE, -1), (FULLY_CONNECTED, 300), (TANH, -1), (FULLY_CONNECTED, 10), (SOFTMAX, -1)]
arhitecture2 = [(LINEARIZE, -1), (FULLY_CONNECTED, 300), (TANH, -1), (FULLY_CONNECTED, 100), (TANH, -1), (FULLY_CONNECTED, 10), (SOFTMAX, -1)]
arhitecture3 = [(LINEARIZE, -1), (FULLY_CONNECTED, 900), (TANH, -1), (FULLY_CONNECTED, 300), (TANH, -1), (FULLY_CONNECTED, 100), (TANH, -1), (FULLY_CONNECTED, 10), (SOFTMAX, -1)]

index = 0
for i in (arhitecture1, arhitecture2, arhitecture3):
	index += 1
	(inputTrainC, outputTrainC, inputTestC, outputTestC) = run(cifar, i, 0.002, 4000, 20000)
	x = inputTestC
	y = outputTestC
	plot(x, y, label="Arhitectura " + str(index))

legend(prop={'size':6})
show()