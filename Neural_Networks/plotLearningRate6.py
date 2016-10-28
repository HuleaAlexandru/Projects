#!/usr/bin/python

from pylab import plot, show, arange, legend, xlabel, ylabel
from train import run
from data_loader_mnist import load_mnist
from data_loader_cifrar import load_cifrar
from feed_forward import LINEARIZE, FULLY_CONNECTED, TANH, SOFTMAX, RELU, CONV, MAX_POOLING

xlabel('Numarul de imagini de antrenare')
ylabel('Acuratete Teste')

mnist = load_mnist()
arhitecture = [(CONV, (6, 14, 14), 6, 2), (RELU, -1), (MAX_POOLING, (6, 7, 7)), (LINEARIZE, -1), (FULLY_CONNECTED, 49), (FULLY_CONNECTED, 10), (SOFTMAX, -1)]

for i in (0.001, 0.002):
	(inputTrainM, outputTrainM, inputTestM, outputTestM) = run(mnist, arhitecture, i, 2000, 10000)
	x = inputTestM
	y = outputTestM
	plot(x, y, label="Learning_rate = " + str(i))

legend(prop={'size':6})
show()