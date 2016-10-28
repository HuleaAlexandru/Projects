import numpy as np
from layer import Layer
from transfer_functions import maxPoolingFunction

class MaxPooling(Layer):

	def __init__(self, inputs_no, outputs_no):
		self.inputs_no = inputs_no
		self.outputs_no = outputs_no
		self.stride = inputs_no[len(inputs_no) - 1] / outputs_no[len(outputs_no) - 1]
		self.f = maxPoolingFunction

		self.outputs = np.zeros(self.outputs_no)
		# Gradients
		self.yGradient = np.zeros(self.outputs_no)
		self.xGradient = np.zeros(self.inputs_no)

	def forward(self, inputs):
		assert(inputs.shape == self.inputs_no)
		# -> self.outputs
		(self.outputs, self.ids) = self.f(inputs, self.stride)
		return self.outputs

	def backward(self, inputs, output_errors):
		assert(output_errors.shape == self.outputs_no)
		self.yGradient = output_errors * self.f(self.outputs, self.stride, True)
		for m in range(self.inputs_no[0]):
			for i in range(self.inputs_no[1]):
				for j in range(self.inputs_no[2]):
					if (inputs[m][i][j] == self.outputs[m][i / self.stride][j / self.stride]):
						self.xGradient[m][i][j] = self.yGradient[m][i / self.stride][j / self.stride]
					else:
						self.xGradient[m][i][j] = 0
		# Compute and return the gradients w.r.t the inputs of this layer
		return self.xGradient

	def zeroGradients(self):
		self.yGradient = np.zeros(self.outputs_no)
		self.xGradient = np.zeros(self.inputs_no)

	def updateParameters(self, learning_rate):
		pass

	def to_string(self):
		return ("(%s -> %s)" % (self.inputs_no, self.outputs_no))