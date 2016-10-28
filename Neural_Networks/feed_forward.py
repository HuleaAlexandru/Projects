import numpy as np
from linearize import Linearize
from tanh import TanH
from conv import Conv
from relu import ReLU
from maxPooling import MaxPooling
from fullyConnected import FullyConnected
from softMax import SoftMax

LINEARIZE = 0
FULLY_CONNECTED = 1
TANH = 2
SOFTMAX = 3
CONV = 4
MAX_POOLING = 5
RELU = 6

class FeedForward:
	def __init__(self, input_size, layers_type):
		self.layers = []
		last_size = input_size
		for type in layers_type:
			if (type[0] == LINEARIZE):
				dim = 1
				for i in range(len(last_size)):
					dim = dim * last_size[i]
				self.layers.append(Linearize(last_size, dim))
				last_size = dim
			elif (type[0] == TANH):
				self.layers.append(TanH(last_size, last_size))
			elif (type[0] == FULLY_CONNECTED):
				self.layers.append(FullyConnected(last_size, type[1]))
				last_size = type[1]
			elif (type[0] == SOFTMAX):
				self.layers.append(SoftMax(last_size, last_size))
			elif (type[0] == CONV):
				self.layers.append(Conv(last_size, type[1], type[2], type[3]))
				last_size = type[1]
			elif (type[0] == MAX_POOLING):
				self.layers.append(MaxPooling(last_size, type[1]))
				last_size = type[1]
			elif (type[0] == RELU):
				self.layers.append(ReLU(last_size, last_size))


	def forward(self, inputs):
		last_input = inputs
		for layer in self.layers:
			last_input = layer.forward(last_input)
		return last_input

	def backward(self, inputs, output_error):
		crt_error = output_error
		for layer_no in range(len(self.layers)-1, 0, -1):
			crt_layer = self.layers[layer_no]
			prev_layer = self.layers[layer_no - 1]
			crt_error = crt_layer.backward(prev_layer.outputs, crt_error)
		self.layers[0].backward(inputs, crt_error)

	def update_parameters(self, learning_rate):
		for l in self.layers:
			l.updateParameters(learning_rate)

	def zero_gradients(self):
		for l in self.layers:
			l.zeroGradients()

	def to_string(self):
		return " -> ".join(map(lambda l: l.to_string(), self.layers))
