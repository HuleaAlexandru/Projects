import numpy as np
from layer import Layer
from transfer_functions import softMaxFunction

class SoftMax(Layer):

	def __init__(self, inputs_no, outputs_no):
		self.inputs_no = inputs_no
		self.outputs_no = outputs_no
		self.f = softMaxFunction
		self.outputs = np.zeros((self.outputs_no, 1))
		# Gradients
		self.yGradient = np.zeros(self.outputs_no)
		self.xGradient = np.zeros(self.inputs_no)

	def forward(self, inputs):
		assert(inputs.shape == (self.inputs_no, 1))
		# -> self.outputs
		self.outputs = self.f(inputs)
		return self.outputs

	def backward(self, inputs, output_errors):
		assert(output_errors.shape == (self.outputs_no, 1))
		self.yGradient = output_errors * self.f(self.outputs, True)
		val = (self.yGradient * self.outputs).sum()
		Z = np.ones((self.outputs_no, 1)) * val
		self.xGradient = self.outputs * (self.yGradient - Z)
		# Compute and return the gradients w.r.t the inputs of this layer
		return self.xGradient

	def zeroGradients(self):
		self.yGradient = np.zeros(self.outputs_no)
		self.xGradient = np.zeros(self.inputs_no)

	def updateParameters(self, learning_rate):
		pass

	def to_string(self):
		return ("(%s -> %s)" % (self.inputs_no, self.outputs_no))