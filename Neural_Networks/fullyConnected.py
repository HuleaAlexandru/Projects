import numpy as np
from layer import Layer
from transfer_functions import identity
from copy import deepcopy

class FullyConnected(Layer):

	def __init__(self, inputs_no, outputs_no):
		# Number of inputs, number of outputs, and the transfer function
		self.inputs_no = inputs_no
		self.outputs_no = outputs_no
		self.f = identity
		# Layer's parameters
		self.weights = np.random.normal(
			0,
			np.sqrt(2.0 / float(self.outputs_no + self.inputs_no)),
			(self.outputs_no, self.inputs_no)
		)
		self.biases = np.random.normal(
			0,
			np.sqrt(2.0 / float(self.outputs_no + self.inputs_no)),
			(self.outputs_no, 1)
		)
		# Computed values
		self.a = np.zeros((self.outputs_no, 1))
		self.outputs = np.zeros((self.outputs_no, 1))
		# Gradients
		self.g_weights = np.zeros((self.outputs_no, self.inputs_no))
		self.g_biases = np.zeros((self.outputs_no, 1))
		self.yGradient = np.zeros(self.outputs_no)
		self.xGradient = np.zeros(self.inputs_no)

	def forward(self, inputs):
		assert(inputs.shape == (self.inputs_no, 1))
		# -> compute self.a and self.outputs
		self.a = np.dot(self.weights, inputs) + self.biases
		self.outputs = self.f(self.a)
		return self.outputs

	def backward(self, inputs, output_errors):
		assert(output_errors.shape == (self.outputs_no, 1))
		self.yGradient = output_errors * self.f(self.outputs, True)
		# Compute the gradients w.r.t. the bias terms (self.g_biases)
		self.g_biases = deepcopy(self.yGradient)
		# Compute the gradients w.r.t. the weights (self.g_weights)
		self.g_weights = np.dot(deepcopy(self.yGradient), inputs.T)
		# Compute and return the gradients w.r.t the inputs of this layer
		self.xGradient = np.dot(self.weights.T, deepcopy(self.yGradient))
		return self.xGradient

	def zeroGradients(self):
		self.g_weights = np.zeros((self.outputs_no, self.inputs_no))
		self.g_biases = np.zeros((self.outputs_no, 1))
		self.yGradient = np.zeros(self.outputs_no)
		self.xGradient = np.zeros(self.inputs_no)

	def updateParameters(self, learning_rate):
		self.weights = self.weights - learning_rate * self.g_weights
		self.biases = self.biases - learning_rate * self.g_biases

	def to_string(self):
		return ("(%s -> %s)" % (self.inputs_no, self.outputs_no))