#!/usr/bin/python

from pylab import plot, show, arange, legend, xlabel, ylabel
from train import run
from data_loader_mnist import load_mnist
from data_loader_cifrar import load_cifrar
from feed_forward import LINEARIZE, FULLY_CONNECTED, TANH, SOFTMAX

xlabel('Numarul de imagini de antrenare')
ylabel('Acuratete Teste')

mnist = load_mnist()
arhitecture = [(LINEARIZE, -1), (FULLY_CONNECTED, 300), (TANH, -1), (FULLY_CONNECTED, 100), (TANH, -1), (FULLY_CONNECTED, 10), (SOFTMAX, -1)]

for i in (0.001, 0.002, 0.004):
	(inputTrainM, outputTrainM, inputTestM, outputTestM) = run(mnist, arhitecture, i, 4000, 20000)
	x = inputTestM
	y = outputTestM
	plot(x, y, label="Learning_rate = " + str(i))

legend(prop={'size':6})
show()