import numpy as np
from layer import Layer
from transfer_functions import reluFunction

class ReLU(Layer):

	def __init__(self, inputs_no, outputs_no):
		self.inputs_no = inputs_no
		self.outputs_no = outputs_no
		self.f = reluFunction
		self.outputs = np.zeros(self.outputs_no)
		# Gradients
		self.yGradient = np.zeros(self.outputs_no)
		self.xGradient = np.zeros(self.inputs_no)

	def forward(self, inputs):
		assert(inputs.shape == self.inputs_no)
		# -> self.outputs
		self.outputs = self.f(inputs)
		return self.outputs

	def backward(self, inputs, output_errors):
		assert(output_errors.shape == self.outputs_no)
		self.yGradient = output_errors * self.f(self.outputs, True)
		(m,h,w) = self.outputs_no
		for i in range(m):
			for j in range(h):
				for k in range(w):
					if (inputs[i][j][k] > 0):
						self.xGradient[i][j][k] = self.yGradient[i][j][k] 
		# Compute and return the gradients w.r.t the inputs of this layer
		return self.xGradient

	def zeroGradients(self):
		self.yGradient = np.zeros(self.outputs_no)
		self.xGradient = np.zeros(self.inputs_no)

	def updateParameters(self, learning_rate):
		pass

	def to_string(self):
		return ("(%s -> %s)" % (self.inputs_no, self.outputs_no))