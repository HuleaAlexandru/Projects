import numpy as np
from layer import Layer
from transfer_functions import hyperbolic_tangent

class Linearize(Layer):

	def __init__(self, inputs_no, outputs_no):
		self.inputs_no = inputs_no
		self.outputs_no = outputs_no
		self.outputs = np.zeros(self.outputs_no)

	def forward(self, inputs):
		assert(inputs.shape == self.inputs_no)

		# -> self.outputs
		dim = 1
		for i in range(len(self.inputs_no)):
			dim = dim * self.inputs_no[i]

		assert(dim == self.outputs_no)

		self.outputs = inputs.reshape(self.outputs_no, 1)

		return self.outputs

	def backward(self, inputs, output_errors):
		assert(output_errors.shape == (self.outputs_no, 1))

		# Compute and return the gradients w.r.t the inputs of this layer
		return output_errors.reshape(*self.inputs_no)

	def zeroGradients(self):
		pass

	def updateParameters(self, learning_rate):
		return

	def to_string(self):
		return ("(%s -> %s)" % (self.inputs_no, self.outputs_no))