import numpy as np
from layer import Layer
from transfer_functions import identity
from copy import deepcopy

class Conv(Layer):

	def __init__(self, inputs_no, outputs_no, k, s):
		# Number of inputs, number of outputs, and the transfer function
		self.inputs_no = inputs_no
		self.outputs_no = outputs_no
		self.f = identity
		self.k = k
		self.s = s
		
		inDim = np.prod(inputs_no)
		outDim = np.prod(outputs_no)

		# Layer's parameters
		self.weights = np.random.normal(
			0,
			np.sqrt(2.0 / float(outDim + inDim)),
			(outputs_no[0], inputs_no[0], self.k, self.k)
		)
		self.biases = np.random.normal(
			0,
			np.sqrt(2.0 / float(outDim + inDim)),
			(self.outputs_no[0], 1)
		)
		# Computed values
		self.a = np.zeros(self.outputs_no)
		self.outputs = np.zeros(self.outputs_no)
		# Gradients
		self.g_weights = np.zeros((self.outputs_no[0], self.inputs_no[0], self.k, self.k))
		self.g_biases = np.zeros(self.outputs_no[0])
		self.yGradient = np.zeros(self.outputs_no)
		self.xGradient = np.zeros(self.inputs_no)
		
	def forward(self, inputs):
		assert(inputs.shape == self.inputs_no)
		# -> compute self.a and self.outputs
		for n in range(self.outputs_no[0]):
			for i in range(self.outputs_no[1]):
				for j in range(self.outputs_no[2]):
					suma = 0
					for m in range(self.inputs_no[0]):
						for p in range(self.k):
							for q in range(self.k):
								suma += self.weights[n][m][p][q] * inputs[m][i * self.s + p][j * self.s + q]
					self.a[n][i][j] = suma + self.biases[n][0]

		self.outputs = self.f(self.a)
		return self.outputs

	def backward(self, inputs, output_errors):
		assert(output_errors.shape == self.outputs_no)
		self.yGradient = output_errors * self.f(self.outputs, True)
		# Compute the gradients w.r.t. the bias terms (self.g_biases)
		for i in range(self.outputs_no[0]):
			self.g_biases[i] = np.mean(self.yGradient[i])
		# Compute the gradients w.r.t. the weights (self.g_weights)
		for n in range(self.outputs_no[0]):
			for m in range(self.inputs_no[0]):
				for p in range(self.k):
					for q in range(self.k):
						sum = 0
						for i in range(self.outputs_no[1]):
							for j in range(self.outputs_no[2]):
								sum += inputs[m][i * self.s + p][j * self.s + q] * self.yGradient[n][i][j]
						self.g_weights[n][m][p][q] = sum

		for m in range(self.inputs_no[0]):
			for i in range(self.inputs_no[1]):
				for j in range(self.inputs_no[2]):	
					sum = 0
					for n in range(self.outputs_no[0]):
						for p in range(self.k):
							if (i < p or (i - p) / self.s >= self.outputs_no[1]):
								continue
							for q in range(self.k):
								if (j < q or (j - q) / self.s >= self.outputs_no[2]):
									continue
								sum += self.weights[n][m][p][q] * self.yGradient[n][(i - p) / self.s][(j - q) / self.s]
					self.xGradient[m][i][j] = sum

		# Compute and return the gradients w.r.t the inputs of this layer
		return self.xGradient

	def zeroGradients(self):
		self.g_weights = np.zeros((self.outputs_no[0], self.inputs_no[0], self.k, self.k))
		self.g_biases = np.zeros(self.outputs_no[0])
		self.yGradient = np.zeros(self.outputs_no)
		self.xGradient = np.zeros(self.inputs_no)

	def updateParameters(self, learning_rate):
		self.weights = self.weights - learning_rate * self.g_weights
		self.biases = self.biases - learning_rate * self.g_biases

	def to_string(self):
		return ("(%s -> %s)" % (self.inputs_no, self.outputs_no))