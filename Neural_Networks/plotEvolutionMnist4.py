#!/usr/bin/python

from pylab import plot, show, arange, legend, xlabel, ylabel
from train import run
from data_loader_mnist import load_mnist
from data_loader_cifrar import load_cifrar
from feed_forward import LINEARIZE, FULLY_CONNECTED, TANH, SOFTMAX

xlabel('Numarul de imagini de antrenare')
ylabel('Acuratete')

mnist = load_mnist()
cifrar = load_cifrar()
arhitecture = [(LINEARIZE, -1), (FULLY_CONNECTED, 300), (TANH, -1), (FULLY_CONNECTED, 100), (TANH, -1), (FULLY_CONNECTED, 10), (SOFTMAX, -1)]
(inputTrainM, outputTrainM, inputTestM, outputTestM) = run(mnist, arhitecture, 0.002, 4000, 40000)

x = inputTrainM
y = outputTrainM
plot(x, y, color='green', label='MNIST_TRAIN')

x = inputTestM
y = outputTestM
plot(x, y, color='blue', label='MNIST_TEST')

legend(prop={'size':6})
show()